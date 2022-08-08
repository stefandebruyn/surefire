////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <cstdlib>
#include <cmath>

#include "sf/config/StateScriptCompiler.hpp"
#include "sf/core/Assert.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Console.hpp"

/////////////////////////////////// Global /////////////////////////////////////

///
/// @brief State script compiler and runner error text.
///
static const char* const gErrText = "state script error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateScriptCompiler::compile(
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
        return E_SSC_FILE;
    }

    // Set the error info file path for error messages generated further into
    // compilation.
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

    // Send input stream into the next compilation phase.
    return StateScriptCompiler::compile(ifs, kSmAsm, kAsm, kErr);
}

Result StateScriptCompiler::compile(
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
    res = StateScriptParser::parse(toks, parse, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }

        return res;
    }

    // Send input stream into the next compilation phase.
    return StateScriptCompiler::compile(parse, kSmAsm, kAsm, kErr);
}

Result StateScriptCompiler::compile(
    const Ref<const StateScriptParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Ref<StateScriptAssembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Check that parse and state machine assembly are non-null.
    if ((kParse == nullptr) || (kSmAsm == nullptr))
    {
        return E_SSC_NULL;
    }

    // Check that state machine assembly was not raked, otherwise the state
    // script compiler doesn't have the metadata structures it needs to compile.
    if (kSmAsm->mWs.raked)
    {
        return E_SSC_RAKE;
    }

    // Compile state script options.
    StateScriptAssembly::Config config{0, StateMachine::NO_STATE};
    Result res = StateScriptCompiler::compileOptions(kParse->config,
                                                     kSmAsm,
                                                     config,
                                                     kErr);
    if (res != SUCCESS)
    {
        return res;
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
    for (const StateScriptParse::SectionParse& sectionParse : kParse->sections)
    {
        StateScriptAssembly::Section section;

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

            // Get ID of state.
            auto stateIdIt = kSmAsm->mWs.stateIds.find(stateName);
            if (stateIdIt == kSmAsm->mWs.stateIds.end())
            {
                // Unknown state.
                ErrorInfo::set(kErr, sectionParse.tokName, gErrText,
                               ("unknown state `" + stateName + "`"));
                return E_SSC_STATE;
            }
            section.stateId = (*stateIdIt).second;

            // Check that state does not appear in the state script twice.
            if (scriptStates.find(stateName) != scriptStates.end())
            {
                ErrorInfo::set(kErr, sectionParse.tokName, gErrText,
                               ("state `" + stateName
                                + "` has more than one section"));
                return E_SSC_DUPE;
            }
            scriptStates.insert(stateName);
        }

        // If first block in section has no data in it (indicating an empty
        // section), skip this section.
        Ref<const StateMachineParse::BlockParse> block = sectionParse.block;
        SF_SAFE_ASSERT(block != nullptr);
        if ((block->guard == nullptr)
            && (block->action == nullptr)
            && (block->assert == nullptr)
            && (block->tokStop.str.size() == 0))
        {
            // Assert that block is well-formed. Expect it to have no linked
            // blocks.
            SF_SAFE_ASSERT(block->ifBlock == nullptr);
            SF_SAFE_ASSERT(block->elseBlock == nullptr);
            SF_SAFE_ASSERT(block->next == nullptr);

            continue;
        }

        // Compile all blocks in section.
        while (block != nullptr)
        {
            // Check that block has a guard.
            if (block->guard == nullptr)
            {
                // Figure out which token the error message will point to based
                // on the block type.
                Token tokErr;
                if (block->action != nullptr)
                {
                    // Input block.
                    tokErr = block->action->tokLhs;
                }
                else if (block->assert != nullptr)
                {
                    // Assert block.
                    tokErr = block->tokAssert;
                }
                else
                {
                    // Stop block.
                    tokErr = block->tokStop;
                }

                ErrorInfo::set(kErr, tokErr, gErrText, "unguarded statement");
                return E_SSC_GUARD;
            }

            // Check that block has no else branch, which is disallowed in state
            // scripts.
            if (block->elseBlock != nullptr)
            {
                ErrorInfo::set(kErr, block->tokElse, gErrText,
                               ("state scripts may not use `"
                                + LangConst::keywordElse + "`"));
                return E_SSC_ELSE;
            }

            // Compile guard.
            Ref<const ExpressionAssembly> guardAsm;
            res = ExpressionCompiler::compile(block->guard,
                                              kSmAsm->mWs.elems,
                                              ElementType::BOOL,
                                              guardAsm,
                                              kErr);
            if (res != SUCCESS)
            {
                // Override error text set by expression compiler for consistent
                // error messages from state script compiler.
                if (kErr != nullptr)
                {
                    kErr->text = gErrText;
                }

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
                    // Error message will point to first token in guard
                    // expression, or the leftmost leaf in the expression tree.
                    Ref<const ExpressionParse> node = innerBlock->guard;
                    while (node->left != nullptr)
                    {
                        node = node->left;
                    }

                    ErrorInfo::set(kErr, node->data, gErrText,
                                   "state scripts may not use nested guards");
                    return E_SSC_NEST;
                }

                // Check that block is not occurring after a step annotation (in
                // which case it can never execute).
                if (foundSectionStop)
                {
                    // Figure out which token the error message will point to
                    // based on the block type.
                    Token tokErr;
                    if (innerBlock->action != nullptr)
                    {
                        // Input block.
                        tokErr = innerBlock->action->tokLhs;
                    }
                    else if (innerBlock->assert != nullptr)
                    {
                        // Assert block.
                        tokErr = innerBlock->tokAssert;
                    }
                    else
                    {
                        // Stop block.
                        tokErr = innerBlock->tokStop;
                    }

                    ErrorInfo::set(kErr, tokErr, gErrText,
                                   ("statement after `"
                                    + LangConst::annotationStop
                                    + "` can never execute"));
                    return E_SSC_UNRCH;
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

                    // Expect stop block to have no other data. It may have a
                    // a next block, which is still an error (unreachable
                    // statement), but that gets caught later.
                    SF_SAFE_ASSERT(innerBlock->action == nullptr);
                    SF_SAFE_ASSERT(innerBlock->ifBlock == nullptr);
                    SF_SAFE_ASSERT(innerBlock->elseBlock == nullptr);
                    SF_SAFE_ASSERT(innerBlock->assert == nullptr);

                    // Add stop to section with the previously compiled guard.
                    // Stop is represented by an assert with a null assert
                    // expression.
                    StateScriptAssembly::Assert stop
                    {
                        dynamic_cast<IExprNode<bool>*>(guardAsm->root().get()),
                        nullptr,
                        TokenIterator::NONE
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
                    res  = ExpressionCompiler::compile(innerBlock->assert,
                                                       kSmAsm->mWs.elems,
                                                       ElementType::BOOL,
                                                       assertAsm,
                                                       kErr);
                    if (res != SUCCESS)
                    {
                        // Override error text set by expression compiler for
                        // consistent error messages from state script compiler.
                        if (kErr != nullptr)
                        {
                            kErr->text = gErrText;
                        }

                        return res;
                    }
                    exprAsms.push_back(assertAsm);

                    // Add assert to section with the previously compiled guard.
                    StateScriptAssembly::Assert assert
                    {
                        dynamic_cast<IExprNode<bool>*>(guardAsm->root().get()),
                        dynamic_cast<IExprNode<bool>*>(assertAsm->root().get()),
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
                        dynamic_cast<IExprNode<bool>*>(guardAsm->root().get()),
                        nullptr
                    };
                    Ref<const ExpressionAssembly> rhsAsm;
                    res = StateMachineCompiler::compileAssignmentAction(
                        innerBlock->action,
                        kSmAsm->mWs.elems,
                        {},
                        input.action,
                        rhsAsm,
                        kErr);
                    if (res != SUCCESS)
                    {
                        // Override error text set by expression compiler for
                        // consistent error messages from state script compiler.
                        if (kErr != nullptr)
                        {
                            kErr->text = gErrText;
                        }

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
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            kErr->subtext = ("state script has no `" + LangConst::annotationStop
                             + "`");
        }

        return E_SSC_STOP;
    }

    // Create final assembly.
    kAsm.reset(new StateScriptAssembly(sections, kSmAsm, exprAsms, config));

    return SUCCESS;
}

Result StateScriptAssembly::run(ErrorInfo& kTokInfo,
                                StateScriptAssembly::Report& kReport)
{
    Result res = SUCCESS;

    // Zero out the report.
    kReport = {false, 0, 0, ""};

    // Get state machine from assembly.
    SF_SAFE_ASSERT(mSmAsm != nullptr);
    StateMachine& sm = mSmAsm->get();

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

    // Set initial state if one was specified.
    if (mConfig.initState != StateMachine::NO_STATE)
    {
        res = sm.setState(mConfig.initState);
        SF_SAFE_ASSERT(res == SUCCESS);
    }

    // The asserts to run in a given step will be collected in these vectors.
    Vec<StateScriptAssembly::Assert*> activeAsserts;

    // Global time starts at zero.
    elemGlobalTime.write(0);

    // On fail, stores the address of the failed assert.
    const StateScriptAssembly::Assert* failAssert = nullptr;

    // Loop until stop annotation of assert failure.
    while (true)
    {
        // Increment state script step count.
        ++kReport.steps;

        // Update the state elapsed time. Normally this happens when the state
        // machine steps, but we need it to happen slightly earlier so that the
        // value is seen by state script expressions evaluated before stepping.
        U64 stateTime = Clock::NO_TIME;
        SF_SAFE_ASSERT(sm.getStateTime(stateTime) == SUCCESS);
        SF_SAFE_ASSERT(stateTime != Clock::NO_TIME);
        elemStateTime.write(stateTime);

        // Forcibly update the state element, for the same reason as above.
        elemState.write(sm.currentState());

        // Update expression stats for expressions in state script.
        for (const Ref<const ExpressionAssembly> exprAsm : mExprAsms)
        {
            SF_SAFE_ASSERT(exprAsm != nullptr);
            const Vec<Ref<IExpressionStats>> exprStats = exprAsm->stats();
            for (const Ref<IExpressionStats> stat : exprStats)
            {
                SF_SAFE_ASSERT(stat != nullptr);
                stat->update();
            }
        }

        // Execute inputs and collect asserts for the current step based on the
        // current state and guard evaluations.
        for (StateScriptAssembly::Section& section : mSections)
        {
            if ((section.stateId == StateMachine::NO_STATE)
                || (section.stateId == elemState.read()))
            {
                // Execute inputs as we go along so that they are reflected in
                // later guards.
                for (StateScriptAssembly::Input& input : section.inputs)
                {
                    SF_SAFE_ASSERT(input.guard != nullptr);
                    if (input.guard->evaluate())
                    {
                        SF_SAFE_ASSERT(input.action != nullptr);
                        input.action->execute();
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

        // Clear active asserts.
        activeAsserts.clear();

        // Increment global time by the configured delta T.
        SF_SAFE_ASSERT(mConfig.deltaT > 0);
        const U64 lastGlobalTime = elemGlobalTime.read();
        elemGlobalTime.write(lastGlobalTime + mConfig.deltaT);

        // Check for overflow of the global clock.
        if (elemGlobalTime.read() <= lastGlobalTime)
        {
            return E_SSC_OVFL;
        }
    }

    // State script completed- generate report.
    kReport.pass = (failAssert == nullptr);

    // Report text always starts with a "header" that shows the number of steps
    // and passed asserts.
    std::stringstream reportText;
    reportText << "state script ran for " << Console::cyan << kReport.steps
               << Console::reset << ((kReport.steps == 1) ? " step" : " steps")
               << "\n" << Console::green << kReport.asserts << Console::reset
               << ((kReport.asserts == 1) ? " assert" : " asserts")
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
    reportText << "final state vector:\n";
    res = this->printStateVector(reportText);
    if (res != SUCCESS)
    {
        return res;
    }
    kReport.text = reportText.str();

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

Result StateScriptCompiler::compileOptions(
    const StateScriptParse::Config& kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    StateScriptAssembly::Config& kConfig,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kSmAsm != nullptr);

    // Check that a delta T was specified in the options section.
    if (kParse.tokDeltaT.str.size() == 0)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            std::stringstream ss;
            ss << "`" << LangConst::optDeltaT << "` not specified in `"
               << LangConst::sectionOptions << "` section";
            kErr->subtext = ss.str();
        }

        return E_SSC_DT;
    }

    // Convert delta T string to F64.
    const char* const str = kParse.tokDeltaT.str.c_str();
    char* end = nullptr;
    const F64 val = std::strtod(str, &end);

    // Check that conversion succeeded.
    if (end == str)
    {
        ErrorInfo::set(kErr, kParse.tokDeltaT, gErrText, "invalid number");
        return E_SSC_DT;
    }

    // Check that delta T is an integer greater than zero.
    if ((val <= 0.0) || (std::ceil(val) != val))
    {
        ErrorInfo::set(kErr, kParse.tokDeltaT, gErrText,
                       ("`" + LangConst::optDeltaT
                        + "` must be an integer > 0"));
        return E_SSC_DT;
    }

    // Check that delta T is not too large.
    if (val > Limits::max<U64>())
    {
        ErrorInfo::set(kErr, kParse.tokDeltaT, gErrText, "value is too large");
        return E_SSC_DT;
    }

    // Delta T is valid.
    kConfig.deltaT = static_cast<U64>(val);

    // Parse initial state if specified.
    if (kParse.tokInitState.str.size() > 0)
    {
        auto stateIt = kSmAsm->mWs.stateIds.find(kParse.tokInitState.str);
        if (stateIt == kSmAsm->mWs.stateIds.end())
        {
            // Unknown state.
            ErrorInfo::set(kErr, kParse.tokInitState, gErrText,
                           ("unknown state `" + kParse.tokInitState.str + "`"));
            return E_SSC_STATE;
        }

        kConfig.initState = (*stateIt).second;
    }

    return SUCCESS;
}

StateScriptAssembly::StateScriptAssembly(
    const Vec<StateScriptAssembly::Section>& kSections,
    const Ref<const StateMachineAssembly> kSmAsm,
    const Vec<Ref<const ExpressionAssembly>> kExprAsms,
    const StateScriptAssembly::Config& kConfig) :
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
            kOs << "  " << Console::cyan << elemName << Console::reset << " = "
                << Console::cyan;

            // Print element value.
            switch (elemObj->type())
            {
                case ElementType::INT8:
                    kOs << static_cast<I32>(
                        static_cast<const Element<I8>*>(elemObj)->read());
                    break;

                case ElementType::INT16:
                    kOs << static_cast<const Element<I16>*>(elemObj)->read();
                    break;

                case ElementType::INT32:
                    kOs << static_cast<const Element<I32>*>(elemObj)->read();
                    break;

                case ElementType::INT64:
                    kOs << static_cast<const Element<I64>*>(elemObj)->read();
                    break;

                case ElementType::UINT8:
                    kOs << static_cast<I32>(
                        static_cast<const Element<U8>*>(elemObj)->read());
                    break;

                case ElementType::UINT16:
                    kOs << static_cast<const Element<U16>*>(elemObj)->read();
                    break;

                case ElementType::UINT32:
                    kOs << static_cast<const Element<U32>*>(elemObj)->read();
                    break;

                case ElementType::UINT64:
                    kOs << static_cast<const Element<U64>*>(elemObj)->read();
                    break;

                case ElementType::FLOAT32:
                    kOs << std::fixed
                        << static_cast<const Element<F32>*>(elemObj)->read();
                    break;

                case ElementType::FLOAT64:
                    kOs << std::fixed
                        << static_cast<const Element<F64>*>(elemObj)->read();
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
