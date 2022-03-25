#include <fstream>

#include "sf/config/StateScriptAssembly.hpp"
#include "sf/core/Assert.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Console.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state script error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateScriptAssembly::compile(
    const String kFilePath,
    const Ref<const StateMachineAssembly> kSmAsm,
    Ref<StateScriptAssembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Open file input stream.
    std::ifstream ifs(kFilePath);
    if (!ifs.is_open())
    {
        if (kErr != nullptr)
        {
            kErr->text = "error";
            kErr->subtext = "failed to open file `" + kFilePath + "`";
        }
        return E_SMA_FILE;
    }

    // Set the error info file path for error messages generated further into
    // compilation.
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

    // Send input stream into the next compilation phase.
    return StateScriptAssembly::compile(ifs, kSmAsm, kAsm, kErr);
}

Result StateScriptAssembly::compile(
    std::istream& kIs,
    const Ref<const StateMachineAssembly> kSmAsm,
    Ref<StateScriptAssembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Tokenize the input stream.
    Vec<Token> toks;
    Result res = Tokenizer::tokenize(kIs, toks, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }

        return res;
    }

    // Parse the state script.
    Ref<const StateScriptParse> parse;
    res = StateScriptParse::parse(toks, parse, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }

        return res;
    }

    // Send input stream into the next compilation phase.
    return StateScriptAssembly::compile(parse, kSmAsm, kAsm, kErr);
}

Result StateScriptAssembly::compile(
    const Ref<const StateScriptParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Ref<StateScriptAssembly>& kAsm,
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

        // Compile all blocks in section.
        Ref<const StateMachineParse::BlockParse> block = sectionParse.block;
        while (block != nullptr)
        {
            // Check that block has a guard. This is the condition under which the
            // inputs and assertions occur.
            if (block->guard == nullptr)
            {
                SF_ASSERT(false);
            }

            // Check that block has no else branch, which is disallowed in state
            // scripts.
            if (block->elseBlock != nullptr)
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

            // Compile all blocks under guard.
            Ref<const StateMachineParse::BlockParse> innerBlock =
                block->ifBlock;
            while (innerBlock != nullptr)
            {
                // Check that inner block has no nested guards, which are
                // disallowed in state scripts.
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

                // Check that block is well-formed. Expect it to have no if or
                // else branch blocks.
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

                    // Add stop to section with the previously compiled guard.
                    // Stop is represented by an assert with a null assert
                    // expression.
                    StateScriptAssembly::Assert stop
                    {
                        static_cast<IExprNode<bool>*>(guardAsm->root().get()),
                        nullptr,
                        {}
                    };
                    section.asserts.push_back(stop);
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
                    StateScriptAssembly::Assert assert
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
                    section.inputs.push_back(input);
                }

                // Go to next inner block.
                innerBlock = innerBlock->next;
            }

            // Go to next outer block.
            block = block->next;
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
    kAsm.reset(new StateScriptAssembly(sections,
                                       kSmAsm,
                                       exprAsms,
                                       kParse->config));

    return SUCCESS;
}

Result StateScriptAssembly::run(ErrorInfo& kTokInfo,
                                StateScriptAssembly::Report& kReport)
{
    // Zero out the report.
    kReport = {false, 0, 0, ""};

    // Get state machine from assembly.
    SF_SAFE_ASSERT(mSmAsm != nullptr);
    Ref<StateMachine> smRef = mSmAsm->get();
    SF_SAFE_ASSERT(smRef != nullptr);
    StateMachine& sm = *smRef;

    // Get state time element.
    auto elemIt = mSmAsm->mWs.elems.find("T");
    SF_SAFE_ASSERT(elemIt != mSmAsm->mWs.elems.end());
    IElement* elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);
    SF_SAFE_ASSERT(elemObj->type() == ElementType::UINT64);
    Element<U64>& elemStateTime = *static_cast<Element<U64>*>(elemObj);

    // Get global time element.
    elemIt = mSmAsm->mWs.elems.find("G");
    SF_SAFE_ASSERT(elemIt != mSmAsm->mWs.elems.end());
    elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);
    SF_SAFE_ASSERT(elemObj->type() == ElementType::UINT64);
    Element<U64>& elemGlobalTime = *static_cast<Element<U64>*>(elemObj);

    // Get state element.
    elemIt = mSmAsm->mWs.elems.find("S");
    SF_SAFE_ASSERT(elemIt != mSmAsm->mWs.elems.end());
    elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);
    SF_SAFE_ASSERT(elemObj->type() == ElementType::UINT32);
    Element<U32>& elemState = *static_cast<Element<U32>*>(elemObj);

    // The inputs and asserts to run in a given step will be collected in these
    // vectors.
    Vec<StateScriptAssembly::Input*> activeInputs;
    Vec<StateScriptAssembly::Assert*> activeAsserts;

    // Global time starts at zero.
    elemGlobalTime.write(0);

    // On fail, stores the address of the failed assert.
    const StateScriptAssembly::Assert* failAssert = nullptr;

    // Loop until stop annotation of assert failure.
    Result res = SUCCESS;
    while (true)
    {
        // Increment state script step count.
        ++kReport.steps;

        // Update the state elapsed time. Normally this happens when the state
        // machine steps, but we force it to happen now so that state script
        // guards referencing the state time element behave as expected.
        U64 stateTime = Clock::NO_TIME;
        SF_SAFE_ASSERT(sm.getNextStateTime(stateTime) == SUCCESS);
        SF_SAFE_ASSERT(stateTime != Clock::NO_TIME);
        elemStateTime.write(stateTime);

        // Collect inputs and asserts for the current step based on the current
        // state and guard evaluations.
        for (StateScriptAssembly::Section& section : mSections)
        {
            if ((section.stateId == StateMachine::NO_STATE)
                || (section.stateId == elemState.read()))
            {
                // Collect inputs.
                for (StateScriptAssembly::Input& input : section.inputs)
                {
                    SF_SAFE_ASSERT(input.guard != nullptr);
                    if (input.guard->evaluate())
                    {
                        activeInputs.push_back(&input);
                    }
                }

                // Collect asserts.
                for (StateScriptAssembly::Assert& assert : section.asserts)
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
        for (StateScriptAssembly::Input* const input : activeInputs)
        {
            SF_SAFE_ASSERT(input != nullptr);
            SF_SAFE_ASSERT(input->action != nullptr);
            input->action->execute();
        }

        // Step state machine.
        sm.step();

        // Evaluate asserts.
        bool stop = false;
        for (StateScriptAssembly::Assert* const assert : activeAsserts)
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

            // Assert passed.
            ++kReport.asserts;
        }

        // Break loop when an assert fails or stop is encountered.
        if ((failAssert != nullptr) || stop)
        {
            break;
        }

        // Clear active inputs and asserts.
        activeInputs.clear();
        activeAsserts.clear();

        // Increment global time by the configured delta T.
        SF_SAFE_ASSERT(mConfig.deltaT > 0);
        const U64 lastGlobalTime = elemGlobalTime.read();
        elemGlobalTime.write(lastGlobalTime + mConfig.deltaT);

        // Check for overflow of the global clock.
        if (elemGlobalTime.read() <= lastGlobalTime)
        {
            SF_ASSERT(false);
        }
    }

    // State script completed- generate report.
    kReport.pass = (failAssert == nullptr);

    // Report text always starts with a "header" that shows the number of steps
    // and passed asserts.
    std::stringstream reportText;
    reportText << "state script ran for " << Console::cyan << kReport.steps
               << Console::reset << ((kReport.steps == 1) ? "step" : "steps")
               << "\n" << Console::green << kReport.asserts << Console::reset
               << ((kReport.asserts == 1) ? "assert" : "asserts")
               << " passed\n";

    // If an assert failed, include an error message using the error info from
    // the tokenization step. This error info contains the original plaintext
    // of the state script, so we can point to the exact location of the failed
    // assert.
    if (failAssert != nullptr)
    {
        kTokInfo.text = "assertion failure";
        kTokInfo.subtext = "assertion failed";
        kTokInfo.lineNum = failAssert->tokAssert.lineNum;
        kTokInfo.colNum = failAssert->tokAssert.colNum;
        reportText << kTokInfo.prettifyError() << "\n";
    }

    // Conclude report text with the final state vector.
    res = this->printStateVector(reportText);
    if (res != SUCCESS)
    {
        return res;
    }
    kReport.text = reportText.str();

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

StateScriptAssembly::StateScriptAssembly(
    const Vec<StateScriptAssembly::Section>& kSections,
    const Ref<const StateMachineAssembly> kSmAsm,
    const Vec<Ref<const ExpressionAssembly>> kExprAsms,
    const StateScriptParse::Config& kConfig) :
    mSections(kSections), mSmAsm(kSmAsm), mExprAsms(kExprAsms), mConfig(kConfig)
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

        // Print element if not already printed.
        if (printedElems.find(elemObj) == printedElems.end())
        {
            // Print element name and equal sign.
            kOs << "    " << Console::cyan << elemName << Console::reset
                << " = " << Console::cyan;

            // Print element value.
            switch (elemObj->type())
            {
                case ElementType::INT8:
                    kOs << static_cast<I32>(
                        static_cast<const Element<I8>*>(elemObj)->read());
                    break;

                case ElementType::INT16:
                    kOs <<
                        static_cast<const Element<I16>*>(elemObj)->read();
                    break;

                case ElementType::INT32:
                    kOs <<
                        static_cast<const Element<I32>*>(elemObj)->read();
                    break;

                case ElementType::INT64:
                    kOs <<
                        static_cast<const Element<I64>*>(elemObj)->read();
                    break;

                case ElementType::UINT8:
                    kOs << static_cast<I32>(
                        static_cast<const Element<U8>*>(elemObj)->read());
                    break;

                case ElementType::UINT16:
                    kOs <<
                        static_cast<const Element<U16>*>(elemObj)->read();
                    break;

                case ElementType::UINT32:
                    kOs <<
                        static_cast<const Element<U32>*>(elemObj)->read();
                    break;

                case ElementType::UINT64:
                    kOs <<
                        static_cast<const Element<U64>*>(elemObj)->read();
                    break;

                case ElementType::FLOAT32:
                    kOs <<
                        static_cast<const Element<F32>*>(elemObj)->read();
                    break;

                case ElementType::FLOAT64:
                    kOs <<
                        static_cast<const Element<F64>*>(elemObj)->read();
                    break;

                case ElementType::BOOL:
                    if (static_cast<const Element<bool>*>(elemObj)->read())
                    {
                        kOs << LangConst::constantTrue;
                    }
                    else
                    {
                        kOs << LangConst::constantFalse;
                    }
                    break;

                default:
                    // Unreachable.
                    SF_SAFE_ASSERT(false);
            }

            // Reset console color and cap line with a newline.
            kOs << Console::reset << std::endl;
        }

        // Add element to printed set.
        printedElems.insert(elemObj);
    }

    return SUCCESS;
}
