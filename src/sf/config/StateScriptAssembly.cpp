#include "sf/config/StateScriptAssembly.hpp"
#include "sf/core/Assert.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Console.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

extern const char* const gErrText = "state script error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateScriptAssembly::compile(
    const Ref<const StateScriptParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Ref<const StateScriptAssembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Check that parse and state machine assembly are non-null.
    if ((kParse == nullptr) || (kSmAsm == nullptr))
    {
        SF_ASSERT(false);
    }

    // Vector to collect compiled sections in.
    Vec<StateScriptAssembly::Section> sections;

    // Vector to collect expression assemblies in.
    Vec<Ref<const ExpressionAssembly>> exprAsms;

    // Names of states with a section in the state script.
    Set<String> scriptStates;

    // Flag for if a stop annotation has been encountered in any section.
    bool foundScriptStop = false;

    // Compile all sections.
    Result res = SUCCESS;
    for (const StateScriptParse::SectionParse& sectionParse : kParse->sections)
    {
        StateScriptAssembly::Section section{};
        Ref<const StateMachineParse::BlockParse> block = sectionParse.block;

        if (sectionParse.tokName.str == LangConst::sectionAllStates)
        {
            // Section will execute in all states.
            section.stateId = StateMachine::NO_STATE;
        }
        else
        {
            // Get state name by stripping the brackets off the section name.
            const String& sectionName = sectionParse.tokName.str;
            SF_SAFE_ASSERT(sectionName.size() >= 3);
            const String stateName =
                sectionName.substr(1, (sectionName.size() - 2));

            // Check that state does not appear in the state script twice.
            if (scriptStates.find(stateName) != scriptStates.end())
            {
                SF_ASSERT(false);
            }
            scriptStates.insert(stateName);

            // Get ID of state.
            auto stateIdIt = kSmAsm->mWs.stateIds.find(stateName);
            if (stateIdIt == kSmAsm->mWs.stateIds.end())
            {
                // Unknown state.
                SF_ASSERT(false);
            }
            section.stateId = (*stateIdIt).second;
        }

        // Check that block has a guard. This is the condition under which the
        // inputs and assertions occur.
        if (block->guard == nullptr)
        {
            SF_ASSERT(false);
        }

        // Compile guard.
        Ref<const ExpressionAssembly> guardAsm;
        res = ExpressionAssembly::compile(block->guard,
                                          kSmAsm->mWs.elems,
                                          ElementType::BOOL,
                                          guardAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            return res;
        }
        exprAsms.push_back(guardAsm);

        // Flag for if a stop annotation has been encountered in this section.
        bool foundSectionStop = false;

        // Compile all blocks in section.
        Ref<const StateMachineParse::BlockParse> innerBlock = block->ifBlock;
        while (innerBlock != nullptr)
        {
            // Check that inner block has no nested guards, which are disallowed
            // in state scripts.
            if (innerBlock->guard != nullptr)
            {
                SF_ASSERT(false);
            }

            // Check that block is not occurring after a step annotation (in
            // which case it can never execute).
            if (foundSectionStop)
            {
                SF_ASSERT(false);
            }

            // Check that block is well-formed. Expect it to have no if or else
            // branch blocks.
            SF_SAFE_ASSERT(innerBlock->ifBlock == nullptr);
            SF_SAFE_ASSERT(innerBlock->elseBlock == nullptr);

            if (innerBlock->tokStop.str.size() > 0)
            {
                // Block is a stop annotation.
                foundSectionStop = true;
                foundScriptStop = true;

                // Expect stop block to have no other data.
                SF_SAFE_ASSERT(innerBlock->action == nullptr);
                SF_SAFE_ASSERT(innerBlock->ifBlock == nullptr);
                SF_SAFE_ASSERT(innerBlock->elseBlock == nullptr);
                SF_SAFE_ASSERT(innerBlock->next == nullptr);
                SF_SAFE_ASSERT(innerBlock->assert == nullptr);

                // Stop is represented as an all-null assert.
                section.asserts.push_back({nullptr, nullptr, {}});
            }
            else if (innerBlock->assert != nullptr)
            {
                // Block is an assert.

                // Expect assert block to not have an action.
                SF_SAFE_ASSERT(innerBlock->action == nullptr);

                // Compile assert expression.
                Ref<const ExpressionAssembly> assertAsm;
                res  = ExpressionAssembly::compile(innerBlock->assert,
                                                   kSmAsm->mWs.elems,
                                                   ElementType::BOOL,
                                                   assertAsm,
                                                   kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
                exprAsms.push_back(assertAsm);

                // Add assert to section with the previously compiled guard.
                const StateScriptAssembly::Assert assert
                {
                    static_cast<IExprNode<bool>*>(guardAsm->root().get()),
                    static_cast<IExprNode<bool>*>(assertAsm->root().get()),
                    innerBlock->tokAssert
                };
                section.asserts.push_back(assert);
            }
            else
            {
                // Block is an input.

                // Expect input block to have an action.
                SF_SAFE_ASSERT(innerBlock->action != nullptr);

                // Compile input.
                StateScriptAssembly::Input input
                {
                    static_cast<IExprNode<bool>*>(guardAsm->root().get()),
                    nullptr
                };
                Ref<const ExpressionAssembly> rhsAsm;
                res = StateMachineAssembly::compileAssignmentAction(
                    innerBlock->action,
                    kSmAsm->mWs.elems,
                    kSmAsm->mWs.readOnlyElems,
                    input.action,
                    rhsAsm,
                    kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
                exprAsms.push_back(rhsAsm);
            }

            // Go to next block.
            innerBlock = innerBlock->next;
        }

        // Add section to assembly.
        sections.push_back(section);
    }

    // Check that a stop annotation was found (if not, the state script can
    // never exit).
    if (!foundScriptStop)
    {
        SF_ASSERT(false);
    }

    // Create final assembly.
    kAsm.reset(new StateScriptAssembly(sections, kSmAsm, exprAsms));

    return SUCCESS;
}

Result StateScriptAssembly::run(bool& kPass,
                                ErrorInfo& kParseInfo,
                                std::ostream& kOs)
{
    // Get state machine from assembly.
    SF_SAFE_ASSERT(mSmAsm != nullptr);
    Ref<StateMachine> smRef = mSmAsm->get();
    SF_SAFE_ASSERT(smRef != nullptr);
    StateMachine& sm = *smRef;

    // Get state element.
    auto elemIt = mSmAsm->mWs.elems.find("S");
    SF_SAFE_ASSERT(elemIt != mSmAsm->mWs.elems.end());
    IElement* elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);
    SF_SAFE_ASSERT(elemObj->type() == ElementType::UINT32);
    Element<U32>& elemState = *static_cast<Element<U32>*>(elemObj);

    // Get state time element.
    elemIt = mSmAsm->mWs.elems.find("T");
    SF_SAFE_ASSERT(elemIt != mSmAsm->mWs.elems.end());
    elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);
    SF_SAFE_ASSERT(elemObj->type() == ElementType::UINT64);
    Element<U64>& elemStateTime = *static_cast<Element<U64>*>(elemObj);

    (void) elemStateTime;

    // The inputs and asserts to run in a given step will be collected in these
    // vectors.
    Vec<const StateScriptAssembly::Input*> activeInputs;
    Vec<const StateScriptAssembly::Assert*> activeAsserts;

    // Number of assert passes.
    U32 assertPasses = 0;

    // Loop until stop annotation of assert failure.
    Result res = SUCCESS;
    while (true)
    {
        // Update the state elapsed time. Normally this happens when the state
        // machine steps, but we force it to happen now so that state script
        // guards referencing the state time element behave as expected.
        U64 stateTime = Clock::NO_TIME;
        SF_SAFE_ASSERT(sm.getNextStateTime(stateTime));
        SF_SAFE_ASSERT(stateTime != Clock::NO_TIME);
        elemStateTime.write(stateTime);

        // Collect inputs and asserts for the current step based on the current
        // state and guard evaluations.
        for (const StateScriptAssembly::Section& section : mSections)
        {
            if ((section.stateId == StateMachine::NO_STATE)
                || (section.stateId == elemState.read()))
            {
                // Collect inputs.
                for (const StateScriptAssembly::Input& input : section.inputs)
                {
                    SF_SAFE_ASSERT(input.guard != nullptr);
                    if (input.guard->evaluate())
                    {
                        activeInputs.push_back(&input);
                    }
                }

                // Collect asserts.
                for (const StateScriptAssembly::Assert& assert :
                     section.asserts)
                {
                    SF_SAFE_ASSERT(assert.guard != nullptr);
                    if (assert.guard->evaluate())
                    {
                        activeAsserts.push_back(&assert);
                    }
                }
            }
        }

        // Execute inputs.
        for (const StateScriptAssembly::Input* const input : activeInputs)
        {
            SF_SAFE_ASSERT(input != nullptr);
            SF_SAFE_ASSERT(input->action != nullptr);
            input->action->execute();
        }

        // Step state machine.
        sm.step();

        // Evaluate asserts.
        const StateScriptAssembly::Assert* failAssert = nullptr;
        bool stop = false;
        for (const StateScriptAssembly::Assert* const assert : activeAsserts)
        {
            SF_SAFE_ASSERT(assert != nullptr);

            // Null assert indicates a stop.
            if (assert->assert == nullptr)
            {
                stop = true;
                break;
            }

            SF_SAFE_ASSERT(assert->assert != nullptr);
            if (!assert->assert->evaluate())
            {
                failAssert = assert;
                break;
            }

            ++assertPasses;
        }

        // Break loop when stop is encountered.
        if (stop)
        {
            break;
        }

        // On assert fail, print report and return.
        if (failAssert != nullptr)
        {
            // Print error message using the error info from the original state
            // script parse, so that we can point to the failing assert token.
            kParseInfo.text = "assertion failure";
            kParseInfo.subtext = "assertion failed";
            kParseInfo.lineNum = failAssert->tokAssert.lineNum;
            kParseInfo.colNum = failAssert->tokAssert.colNum;
            kOs << kParseInfo.prettifyError() << std::endl;

            // Print final state.
            kOs << "final state vector:\n";
            res = this->printStateVector(kOs);
            if (res != SUCCESS)
            {
                return res;
            }

            // Return failure.
            kPass = false;
            return SUCCESS;
        }

        // Clear active inputs and asserts.
        activeInputs.clear();
        activeAsserts.clear();
    }

    // If we got this far, the state script passed. Print report.
    kOs << Console::green << assertPasses << " assert(s) passed"
        << Console::reset << "\n";
    res = this->printStateVector(kOs);
    if (res != SUCCESS)
    {
        return res;
    }

    kPass = true;
    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

StateScriptAssembly::StateScriptAssembly(
    const Vec<StateScriptAssembly::Section>& kSections,
    const Ref<const StateMachineAssembly> kSmAsm,
    const Vec<Ref<const ExpressionAssembly>> kExprAsms) :
    mSections(kSections), mSmAsm(kSmAsm), mExprAsms(kExprAsms)
{
}

Result StateScriptAssembly::printStateVector(std::ostream& kOs)
{
    SF_SAFE_ASSERT(mSmAsm != nullptr);

    // Addresses of elements printed so far. This is to avoid printing aliased
    // elements twice. The original (non-aliased) element name is printed first
    // since it's the first one added to the element symbol table by the state
    // machine compiler (and the element symbol table is an ordered map).
    Set<const IElement*> printedElems;

    for (auto elem : mSmAsm->mWs.elems)
    {
        // Get element name and object.
        const String elemName = elem.first;
        const IElement* const elemObj = elem.second;

        // Print element name and equal sign.
        kOs << "    " << Console::cyan << elemName << Console::reset << " = "
            << Console::cyan;

        // Print element if not already printed.
        if (printedElems.find(elemObj) == printedElems.end())
        {
            switch (elemObj->type())
            {
                case ElementType::INT8:
                    std::cout <<
                        static_cast<const Element<I8>*>(elemObj)->read();
                    break;

                case ElementType::INT16:
                    std::cout <<
                        static_cast<const Element<I16>*>(elemObj)->read();
                    break;

                case ElementType::INT32:
                    std::cout <<
                        static_cast<const Element<I32>*>(elemObj)->read();
                    break;

                case ElementType::INT64:
                    std::cout <<
                        static_cast<const Element<I64>*>(elemObj)->read();
                    break;

                case ElementType::UINT8:
                    std::cout <<
                        static_cast<const Element<U8>*>(elemObj)->read();
                    break;

                case ElementType::UINT16:
                    std::cout <<
                        static_cast<const Element<U16>*>(elemObj)->read();
                    break;

                case ElementType::UINT32:
                    std::cout <<
                        static_cast<const Element<U32>*>(elemObj)->read();
                    break;

                case ElementType::UINT64:
                    std::cout <<
                        static_cast<const Element<U64>*>(elemObj)->read();
                    break;

                case ElementType::FLOAT32:
                    std::cout <<
                        static_cast<const Element<F32>*>(elemObj)->read();
                    break;

                case ElementType::FLOAT64:
                    std::cout <<
                        static_cast<const Element<F64>*>(elemObj)->read();
                    break;

                case ElementType::BOOL:
                    if (static_cast<const Element<bool>*>(elemObj)->read())
                    {
                        std::cout << LangConst::constantTrue;
                    }
                    else
                    {
                        std::cout << LangConst::constantFalse;
                    }
                    break;

                default:
                    // Unreachable.
                    SF_SAFE_ASSERT(false);
            }
        }

        // Reset console color and cap line with a newline.
        kOs << Console::reset << std::endl;

        // Add element to printed set.
        printedElems.insert(elemObj);
    }

    return SUCCESS;
}
