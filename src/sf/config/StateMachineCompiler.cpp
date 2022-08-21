////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
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

#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/Assert.hpp"

namespace Sf
{

/////////////////////////////////// Global /////////////////////////////////////

///
/// @brief State machine compiler error text.
///
static const char* const gErrText = "state machine config error";

/////////////////////////////////// Public /////////////////////////////////////

const String StateMachineCompiler::FIRST_STATE;

Result StateMachineCompiler::compile(
    const String kFilePath,
    const Ref<const StateVectorAssembly> kSvAsm,
    Ref<const StateMachineAssembly>& kAsm,
    ErrorInfo* const kErr,
    const String kInitState,
    const bool kRake)
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
        return E_SMC_FILE;
    }

    // Set the error info file path for error messages generated further into
    // compilation.
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

    // Send input stream into the next compilation phase.
    return StateMachineCompiler::compile(ifs,
                                         kSvAsm,
                                         kAsm,
                                         kErr,
                                         kInitState,
                                         kRake);
}

Result StateMachineCompiler::compile(
    std::istream& kIs,
    const Ref<const StateVectorAssembly> kSvAsm,
    Ref<const StateMachineAssembly>& kAsm,
    ErrorInfo* const kErr,
    const String kInitState,
    const bool kRake)
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

    // Parse the state machine config.
    Ref<const StateMachineParse> parse;
    res = StateMachineParser::parse(toks, parse, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }
        return res;
    }

    // Send state machine parse into the next compilation phase.
    return StateMachineCompiler::compile(parse,
                                         kSvAsm,
                                         kAsm,
                                         kErr,
                                         kInitState,
                                         kRake);
}

Result StateMachineCompiler::compile(
    const Ref<const StateMachineParse> kParse,
    const Ref<const StateVectorAssembly> kSvAsm,
    Ref<const StateMachineAssembly>& kAsm,
    ErrorInfo* const kErr,
    const String kInitState,
    const bool kRake)
{
    // Check that state machine parse is non-null.
    if (kParse == nullptr)
    {
        return E_SMC_NULL;
    }

    // Initialize a blank workspace for the compilation.
    StateMachineAssembly::Workspace ws;
    ws.raked = false;

    // Put the state machine parse in the workspace so that it can be recalled
    // later.
    ws.smParse = kParse;

    // Put the state vector assembly in the workspace so that it can be recalled
    // later.
    ws.svAsm = kSvAsm;

    // Allocate state machine state config array.
    ws.stateConfigs.reset(new Vec<StateMachine::StateConfig>());

    // Validate the state machine state vector. This will partially populate
    // the element symbol table in the compiler workspace.
    Result res = StateMachineCompiler::checkStateVector(kParse, ws, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile the local state vector. This will complete the element symbol
    // table in the compiler workspace.
    res = StateMachineCompiler::compileLocalStateVector(kParse, ws, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set local element initial values.
    res = StateMachineCompiler::initLocalElementValues(kParse, ws, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Build map of state names to IDs. IDs begin at 1 and count up in the order
    // states are defined in the config.
    for (U32 i = 0; i < kParse->states.size(); ++i)
    {
        const String& tokNameStr = kParse->states[i].tokName.str;
        SF_SAFE_ASSERT(tokNameStr.size() >= 3);
        const String stateName = tokNameStr.substr(1, (tokNameStr.size() - 2));
        ws.stateIds[stateName] = (i + 1);
    }

    // Compile each state machine state.
    for (const StateMachineParse::StateParse& state : kParse->states)
    {
        res = StateMachineCompiler::compileState(state, ws, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Collect expression stats needed by all state machine expressions into a
    // vector.
    Vec<Ref<IExpressionStats>> allExprStats;
    for (const Ref<const ExpressionAssembly> exprAsm : ws.exprAsms)
    {
        const Vec<Ref<IExpressionStats>> exprAsmStats = exprAsm->stats();
        allExprStats.insert(allExprStats.end(),
                            exprAsmStats.begin(),
                            exprAsmStats.end());
    }

    // Allocate array for expression stats pointers.
    ws.exprStatArr.reset(new Vec<IExpressionStats*>());
    for (const Ref<IExpressionStats> exprStats : allExprStats)
    {
        ws.exprStatArr->push_back(exprStats.get());
    }

    // Add expression stats array null terminator required by state machine.
    ws.exprStatArr->push_back(nullptr);

    // Add null terminator to state config vector required by state machine.
    ws.stateConfigs->push_back({StateMachine::NO_STATE,
                                nullptr,
                                nullptr,
                                nullptr});

    // Put together the final state machine config. The config is given the raw
    // pointers underlying the previously allocated state config and expression
    // stats vectors, as well as raw pointers of certain state vector elements.
    SF_SAFE_ASSERT(ws.elems[LangConst::elemState] != nullptr);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemStateTime] != nullptr);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemGlobalTime] != nullptr);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemState]->type()
                   == ElementType::UINT32);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemStateTime]->type()
                   == ElementType::UINT64);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemGlobalTime]->type()
                   == ElementType::UINT64);
    ws.smConfig =
    {
        static_cast<Element<U32>*>(ws.elems[LangConst::elemState]),
        static_cast<Element<U64>*>(ws.elems[LangConst::elemStateTime]),
        static_cast<Element<U64>*>(ws.elems[LangConst::elemGlobalTime]),
        ws.stateConfigs->data(),
        ws.exprStatArr->data()
    };

    // Set initial state as specified.
    SF_SAFE_ASSERT(ws.smConfig.elemState != nullptr);
    if (kInitState == StateMachineCompiler::FIRST_STATE)
    {
        ws.smConfig.elemState->write(1);
    }
    else
    {
        auto stateIdIt = ws.stateIds.find(kInitState);
        if (stateIdIt == ws.stateIds.end())
        {
            // Unknown initial state.
            return E_SMC_INIT;
        }
        const U32 initStateId = (*stateIdIt).second;
        ws.smConfig.elemState->write(initStateId);
    }

    // Create state machine.
    ws.sm.reset(new StateMachine());
    res = StateMachine::init(ws.smConfig, *ws.sm);
    if (res != SUCCESS)
    {
        // Since the state machine config is known correct, the most likely
        // error here is that the user didn't set a valid initial state in the
        // state element.
        return res;
    }

    // If the rake option was specified, clear workspace structures that aren't
    // needed to run the state machine.
    if (kRake)
    {
        ws.elems.clear();
        ws.stateIds.clear();
        ws.readOnlyElems.clear();
        ws.raked = true;
    }

    // Create the final assembly.
    kAsm.reset(new StateMachineAssembly(ws));

    return SUCCESS;
}

StateMachine& StateMachineAssembly::get() const
{
    return *mWs.sm;
}

StateMachine::Config StateMachineAssembly::config() const
{
    return mWs.smConfig;
}

StateVector& StateMachineAssembly::localStateVector() const
{
    return mWs.localSvAsm->get();
}

Ref<const StateMachineParse> StateMachineAssembly::parse() const
{
    return mWs.smParse;
}

/////////////////////////////////// Private ////////////////////////////////////

bool StateMachineCompiler::stateNameReserved(const Token& kTokSection)
{
    return ((kTokSection.str == LangConst::sectionAllStates)
            || (kTokSection.str == LangConst::sectionOptions));
}

Result StateMachineCompiler::checkStateVector(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);
    SF_SAFE_ASSERT(kWs.svAsm != nullptr);

    for (const StateMachineParse::StateVectorElementParse& elem :
         kParse->svElems)
    {
        // Get element object from state vector.
        IElement* elemObj = nullptr;
        StateVector& sv = kWs.svAsm->get();
        if (sv.getIElement(elem.tokName.str.c_str(), elemObj) != SUCCESS)
        {
            // Element does not exist in state vector.
            ErrorInfo::set(kErr, elem.tokName, gErrText,
                           ("element `" + elem.tokName.str
                            + "` does not exist in state vector"));
            return E_SMC_SV_ELEM;
        }
        SF_SAFE_ASSERT(elemObj != nullptr);

        // Look up element type as configured in the state machine.
        auto smTypeInfoIt = TypeInfo::fromName.find(elem.tokType.str);
        if (smTypeInfoIt == TypeInfo::fromName.end())
        {
            // Unknown type.
            ErrorInfo::set(kErr, elem.tokType, gErrText,
                           ("unknown type `" + elem.tokType.str + "`"));
            return E_SMC_TYPE;
        }
        const TypeInfo& smTypeInfo = (*smTypeInfoIt).second;

        // Look up element type info as configured in the actual state vector.
        auto typeInfoIt = TypeInfo::fromEnum.find(elemObj->type());
        SF_SAFE_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& typeInfo = (*typeInfoIt).second;

        // Check that element has the same type in the state vector and state
        // machine.
        if (typeInfo.enumVal != smTypeInfo.enumVal)
        {
            std::stringstream ss;
            ss << "element `" << elem.tokName.str << "` is type "
               << typeInfo.name << " in the state vector but type "
               << smTypeInfo.name << " here";
            ErrorInfo::set(kErr, elem.tokType, gErrText, ss.str());
            return E_SMC_TYPE_MISM;
        }

        // Check that element does not appear twice in the state machine.
        if (kWs.elems.find(elem.tokName.str) != kWs.elems.end())
        {
            ErrorInfo::set(kErr, elem.tokName, gErrText,
                           ("element `" + elem.tokName.str
                            + "` is listed more than once"));
            return E_SMC_ELEM_DUPE;
        }

        // Add element to the symbol table.
        kWs.elems[elem.tokName.str] = elemObj;

        // Make a copy of the element read-only flag. The read-onlyness may
        // change in special cases.
        bool elemReadOnly = elem.readOnly;

        // Check for global time element.
        if ((elem.tokName.str == LangConst::elemGlobalTime)
            || (elem.tokAlias.str == LangConst::elemGlobalTime))
        {
            // Global time element is implicitly read-only.
            elemReadOnly = true;

            // Check that global time element is U64.
            if (smTypeInfo.enumVal != ElementType::UINT64)
            {
                std::stringstream ss;
                ss << "`" << LangConst::elemGlobalTime
                   << "` must be type U64 (" << elem.tokType.str << " here)";
                ErrorInfo::set(kErr, elem.tokName, gErrText, ss.str());
                return E_SMC_G_TYPE;
            }
        }

        // Check for state element.
        if ((elem.tokName.str == LangConst::elemState)
            || (elem.tokAlias.str == LangConst::elemState))
        {
            // State element is implicitly read-only.
            elemReadOnly = true;

            // Check that state element is U32.
            if (smTypeInfo.enumVal != ElementType::UINT32)
            {
                std::stringstream ss;
                ss << "`" << LangConst::elemState << "` must be type U32 ("
                   << elem.tokType.str << " here)";
                ErrorInfo::set(kErr, elem.tokName, gErrText, ss.str());
                return E_SMC_S_TYPE;
            }
        }

        // If the element is aliased, add the alias to the symbol table as well.
        if (elem.tokAlias.str.size() > 0)
        {
            kWs.elems[elem.tokAlias.str] = elemObj;
        }

        // If element is read-only, add its name and alias to read-only set.
        if (elemReadOnly)
        {
            kWs.readOnlyElems.insert(elem.tokName.str);
            if (elem.tokAlias.str.size() > 0)
            {
                kWs.readOnlyElems.insert(elem.tokAlias.str);
            }
        }
    }

    // Check that a global time element was provided.
    if (kWs.elems.find(LangConst::elemGlobalTime) == kWs.elems.end())
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            kErr->subtext = ("no global time element aliased to `"
                             + LangConst::elemGlobalTime + "`");
        }
        return E_SMC_NO_G;
    }

    // Check that a state element was provided.
    if (kWs.elems.find(LangConst::elemState) == kWs.elems.end())
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            kErr->subtext = ("no state element aliased to `"
                             + LangConst::elemState + "`");
        }
        return E_SMC_NO_S;
    }

    return SUCCESS;
}

Result StateMachineCompiler::compileLocalStateVector(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    // The local state vector will be compiled from a manually-built config
    // in a string stream. All elements are configured in a single region.

    // Add region and built-in state time element.
    std::stringstream localSvConfig;
    localSvConfig << LangConst::sectionLocal << "\n"
                  << "U64 " << LangConst::elemStateTime << "\n";

    // State time element is implicitly read-only.
    kWs.readOnlyElems.insert(LangConst::elemStateTime);

    // Add user-configured local elements.
    for (U32 i = 0; i < kParse->localElems.size(); ++i)
    {
        const StateMachineParse::LocalElementParse& elem =
            kParse->localElems[i];

        // Check for name uniqueness against state vector elements and aliases.
        // Uniqueness against local elements will be checked by the state vector
        // compiler later on.
        for (const StateMachineParse::StateVectorElementParse& svElem :
             kParse->svElems)
        {
            if ((elem.tokName.str == svElem.tokName.str)
                || ((svElem.tokAlias.str.size() > 0)
                    && (elem.tokName.str == svElem.tokAlias.str)))
            {
                std::stringstream ss;
                ss << "reuse of element name `" << elem.tokName.str
                   << "` (previously used on line " << svElem.tokName.lineNum
                   << ")";
                ErrorInfo::set(kErr, elem.tokName, gErrText, ss.str());
                return E_SMC_ELEM_DUPE;
            }
        }

        // Add element to local state vector parse.
        localSvConfig << elem.tokType.str << " " << elem.tokName.str << "\n";

        // If element is read-only, add to read-only set.
        if (elem.readOnly)
        {
            kWs.readOnlyElems.insert(elem.tokName.str);
        }
    }

    // Compile the local state vector. We know local state vector config is at
    // least syntatically correct, so the only errors that can occur here would
    // be caused by the user-configured local elements.
    Result res = StateVectorCompiler::compile(localSvConfig,
                                              kWs.localSvAsm,
                                              kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Add local state vector elements to element symbol table.
    SF_SAFE_ASSERT(kWs.localSvAsm != nullptr);
    SF_SAFE_ASSERT(kWs.localSvAsm->parse() != nullptr);
    SF_SAFE_ASSERT(kWs.localSvAsm->parse()->regions.size() == 1);
    for (const StateVectorParse::ElementParse& elem :
         kWs.localSvAsm->parse()->regions[0].elems)
    {
        // Look up element object.
        IElement* elemObj = nullptr;
        res = kWs.localSvAsm->get().getIElement(elem.tokName.str.c_str(),
                                                elemObj);
        SF_SAFE_ASSERT(res == SUCCESS);
        SF_SAFE_ASSERT(elemObj != nullptr);

        // Add element to symbol table.
        kWs.elems[elem.tokName.str] = elemObj;
    }

    return SUCCESS;
}

Result StateMachineCompiler::checkLocalElemInitExprs(
    const StateMachineParse::LocalElementParse& kInitElem,
    const Ref<const ExpressionParse> kExpr,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kWs.svAsm != nullptr);

    // Base case: node is null, so we fell off the tree.
    if (kExpr == nullptr)
    {
        return SUCCESS;
    }

    // If expression node is an element identifier, need to enforce certain
    // rules about the elements that may be referenced.
    if ((kExpr->data.type == Token::IDENTIFIER) && !kExpr->func)
    {
        // Check that element is not being used to initialize itself.
        if (kExpr->data.str == kInitElem.tokName.str)
        {
            ErrorInfo::set(kErr, kExpr->data, gErrText,
                           "cannot use element to initialize itself");
            return E_SMC_SELF_REF;
        }

        // Check that element is not a non-local state vector element.
        IElement* elemObj = nullptr;
        StateVector& sv = kWs.svAsm->get();
        if (sv.getIElement(kExpr->data.str.c_str(), elemObj) == SUCCESS)
        {
            std::stringstream ss;
            ss << "illegal reference to non-local element `"
               << kExpr->data.type << "`";
            ErrorInfo::set(kErr, kExpr->data, gErrText, ss.str());
            return E_SMC_LOC_SV_REF;
        }

        // Check that element is not used before it's initialized.
        SF_SAFE_ASSERT(kWs.smParse != nullptr);
        for (const StateMachineParse::LocalElementParse& elem :
             kWs.smParse->localElems)
        {
            // Break when we run into ourselves in the local element list.
            if (elem.tokName.str == kExpr->data.str)
            {
                break;
            }

            // If we run into the element referenced by this expression node
            // before running into the element being initialized, then that's
            // a use-before-initialization error. Technically this would be
            // well-defined since elements default to zero even before being
            // initialized, but we still consider it unsafe.
            if (elem.tokName.str == kInitElem.tokName.str)
            {
                std::stringstream ss;
                ss << "element `" << kExpr->data.str
                   << "` is not yet initialized";
                ErrorInfo::set(kErr, kExpr->data, gErrText, ss.str());
                return E_SMC_UBI;
            }
        }
    }

    // Check left subtree.
    Result res = StateMachineCompiler::checkLocalElemInitExprs(kInitElem,
                                                               kExpr->left,
                                                               kWs,
                                                               kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Check right subtree.
    res = StateMachineCompiler::checkLocalElemInitExprs(kInitElem,
                                                        kExpr->right,
                                                        kWs,
                                                        kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    return SUCCESS;
}

Result StateMachineCompiler::initLocalElementValues(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);
    SF_SAFE_ASSERT(kWs.localSvAsm != nullptr);

    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        // Validate element references in the initialization expression.
        Result res = StateMachineCompiler::checkLocalElemInitExprs(
            elem, elem.initValExpr, kWs, kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Look up element object so that we can get its type as an enum.
        IElement* const elemObj = kWs.elems[elem.tokName.str];
        SF_SAFE_ASSERT(elemObj != nullptr);

        // Compile element initial value expression.
        Ref<const ExpressionAssembly> initExprAsm;
        res = ExpressionCompiler::compile(elem.initValExpr,
                                          kWs.elems,
                                          elemObj->type(),
                                          initExprAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Evaluate expression and write to element. The element and expression
        // pointers are downcast to template instantiations matching the
        // element's type.
        SF_SAFE_ASSERT(initExprAsm != nullptr);
        IExpression* const iroot = initExprAsm->root().get();
        SF_SAFE_ASSERT(iroot != nullptr);
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT8);
                Element<I8>* const elem = static_cast<Element<I8>*>(elemObj);
                IExprNode<I8>* const root = dynamic_cast<IExprNode<I8>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::INT16:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT16);
                Element<I16>* const elem = static_cast<Element<I16>*>(elemObj);
                IExprNode<I16>* const root =
                    dynamic_cast<IExprNode<I16>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::INT32:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT32);
                Element<I32>* const elem = static_cast<Element<I32>*>(elemObj);
                IExprNode<I32>* const root =
                    dynamic_cast<IExprNode<I32>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::INT64:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT64);
                Element<I64>* const elem = static_cast<Element<I64>*>(elemObj);
                IExprNode<I64>* const root =
                    dynamic_cast<IExprNode<I64>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT8:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT8);
                Element<U8>* const elem = static_cast<Element<U8>*>(elemObj);
                IExprNode<U8>* const root = dynamic_cast<IExprNode<U8>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT16:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT16);
                Element<U16>* const elem = static_cast<Element<U16>*>(elemObj);
                IExprNode<U16>* const root =
                    dynamic_cast<IExprNode<U16>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT32:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT32);
                Element<U32>* const elem = static_cast<Element<U32>*>(elemObj);
                IExprNode<U32>* const root =
                    dynamic_cast<IExprNode<U32>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT64:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT64);
                Element<U64>* const elem = static_cast<Element<U64>*>(elemObj);
                IExprNode<U64>* const root =
                    dynamic_cast<IExprNode<U64>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::FLOAT32:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::FLOAT32);
                Element<F32>* const elem = static_cast<Element<F32>*>(elemObj);
                IExprNode<F32>* const root =
                    dynamic_cast<IExprNode<F32>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::FLOAT64:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::FLOAT64);
                Element<F64>* const elem = static_cast<Element<F64>*>(elemObj);
                IExprNode<F64>* const root =
                    dynamic_cast<IExprNode<F64>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::BOOL:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::BOOL);
                Element<bool>* const elem =
                    static_cast<Element<bool>*>(elemObj);
                IExprNode<bool>* const root =
                    dynamic_cast<IExprNode<bool>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            default:
                // Unreachable.
                SF_SAFE_ASSERT(false);
        }
    }

    return SUCCESS;
}

Result StateMachineCompiler::compileAssignmentAction(
    const Ref<const StateMachineParse::ActionParse> kParse,
    const Map<String, IElement*>& kBindings,
    const Set<String>& kReadOnlyElems,
    Ref<IAction>& kAction,
    Ref<const ExpressionAssembly>& kRhsAsm,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    // Look up LHS element.
    auto elemIt = kBindings.find(kParse->tokLhs.str);
    if (elemIt == kBindings.end())
    {
        // Unknown element.
        ErrorInfo::set(kErr, kParse->tokLhs, gErrText,
                       ("unknown element `" + kParse->tokLhs.str + "`"));
        return E_SMC_ASG_ELEM;
    }
    IElement* const elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);

    // Check that LHS element is not read-only.
    if (kReadOnlyElems.find(kParse->tokLhs.str) != kReadOnlyElems.end())
    {
        ErrorInfo::set(kErr, kParse->tokLhs, gErrText,
                       ("element `" + kParse->tokLhs.str + "` is read-only"));
        return E_SMC_ELEM_RO;
    }

    // Compile RHS expression.
    const Result res = ExpressionCompiler::compile(kParse->rhs,
                                                   kBindings,
                                                   elemObj->type(),
                                                   kRhsAsm,
                                                   kErr);
    if (res != SUCCESS)
    {
        // Override error text set by expression compiler for consistent state
        // machine compiler error messages.
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }

        return res;
    }

    // Create assignment action based on element type. The LHS element object
    // and RHS root node are downcast to template instantiations that match
    // the element type.
    SF_SAFE_ASSERT(kRhsAsm != nullptr);
    SF_SAFE_ASSERT(kRhsAsm->root() != nullptr);
    switch (elemObj->type())
    {
        case ElementType::INT8:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT8);
            kAction.reset(new AssignmentAction<I8>(
                *static_cast<Element<I8>*>(elemObj),
                *dynamic_cast<IExprNode<I8>*>(kRhsAsm->root().get())));
            break;

        case ElementType::INT16:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT16);
            kAction.reset(new AssignmentAction<I16>(
                *static_cast<Element<I16>*>(elemObj),
                *dynamic_cast<IExprNode<I16>*>(kRhsAsm->root().get())));
            break;

        case ElementType::INT32:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT32);
            kAction.reset(new AssignmentAction<I32>(
                *static_cast<Element<I32>*>(elemObj),
                *dynamic_cast<IExprNode<I32>*>(kRhsAsm->root().get())));
            break;

        case ElementType::INT64:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT64);
            kAction.reset(new AssignmentAction<I64>(
                *static_cast<Element<I64>*>(elemObj),
                *dynamic_cast<IExprNode<I64>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT8:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT8);
            kAction.reset(new AssignmentAction<U8>(
                *static_cast<Element<U8>*>(elemObj),
                *dynamic_cast<IExprNode<U8>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT16:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT16);
            kAction.reset(new AssignmentAction<U16>(
                *static_cast<Element<U16>*>(elemObj),
                *dynamic_cast<IExprNode<U16>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT32:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT32);
            kAction.reset(new AssignmentAction<U32>(
                *static_cast<Element<U32>*>(elemObj),
                *dynamic_cast<IExprNode<U32>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT64:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT64);
            kAction.reset(new AssignmentAction<U64>(
                *static_cast<Element<U64>*>(elemObj),
                *dynamic_cast<IExprNode<U64>*>(kRhsAsm->root().get())));
            break;

        case ElementType::FLOAT32:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::FLOAT32);
            kAction.reset(new AssignmentAction<F32>(
                *static_cast<Element<F32>*>(elemObj),
                *dynamic_cast<IExprNode<F32>*>(kRhsAsm->root().get())));
            break;

        case ElementType::FLOAT64:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::FLOAT64);
            kAction.reset(new AssignmentAction<F64>(
                *static_cast<Element<F64>*>(elemObj),
                *dynamic_cast<IExprNode<F64>*>(kRhsAsm->root().get())));
            break;

        case ElementType::BOOL:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::BOOL);
            kAction.reset(new AssignmentAction<bool>(
                *static_cast<Element<bool>*>(elemObj),
                *dynamic_cast<IExprNode<bool>*>(kRhsAsm->root().get())));
            break;

        default:
            // Unreachable.
            SF_SAFE_ASSERT(false);
    }

    return SUCCESS;
}

Result StateMachineCompiler::compileAction(
    const Ref<const StateMachineParse::ActionParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    const bool kInExitLabel,
    Ref<IAction>& kAction,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    Result res = SUCCESS;

    if (kParse->rhs != nullptr)
    {
        // Compile assignment action.
        Ref<const ExpressionAssembly> rhsAsm;
        res = StateMachineCompiler::compileAssignmentAction(kParse,
                                                            kWs.elems,
                                                            kWs.readOnlyElems,
                                                            kAction,
                                                            rhsAsm,
                                                            kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Add compiled RHS expression to the workspace.
        kWs.exprAsms.push_back(rhsAsm);
    }
    else
    {
        // Compile transition action.

        // Check that transition is not in an exit label.
        if (kInExitLabel)
        {
            ErrorInfo::set(kErr, kParse->tokTransitionKeyword, gErrText,
                           "illegal transition in exit label");
            return E_SMC_TR_EXIT;
        }

        // Validate destination state.
        auto stateIdIt = kWs.stateIds.find(kParse->tokDestState.str);
        if (stateIdIt == kWs.stateIds.end())
        {
            ErrorInfo::set(kErr, kParse->tokDestState, gErrText,
                           ("unknown state `" + kParse->tokDestState.str
                            + "`"));
            return E_SMC_STATE;
        }

        // Create transition action with destination state.
        const U32 destState = (*stateIdIt).second;
        kAction.reset(new TransitionAction(destState));
    }

    // Add compiled action to the workspace.
    kWs.actions.push_back(kAction);

    return SUCCESS;
}

Result StateMachineCompiler::compileBlock(
    const Ref<const StateMachineParse::BlockParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    const bool kInExitLabel,
    Ref<StateMachine::Block>& kBlock,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    // Check that an assertion, which is only allowed in state scripts, is not
    // being used in the state machine.
    if (kParse->assert != nullptr)
    {
        ErrorInfo::set(kErr, kParse->tokAssert, gErrText,
                       ("`" + kParse->tokAssert.str
                        + "` may only be used in state scripts"));
        return E_SMC_ASSERT;
    }

    // Check that a stop annotation, which is only allowed in state scripts, is
    // not being used in the state machine.
    if (kParse->tokStop.str.size() > 0)
    {
        ErrorInfo::set(kErr, kParse->tokStop, gErrText,
                       ("`" + kParse->tokStop.str
                        + "` may only be used in state scripts"));
        return E_SMC_STOP;
    }

    // Allocate new block and add to workspace.
    kBlock.reset(new StateMachine::Block{nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr});
    kWs.blocks.push_back(kBlock);

    Result res = SUCCESS;
    if (kParse->guard != nullptr)
    {
        // Compile block guard.
        Ref<const ExpressionAssembly> guardAsm;
        res = ExpressionCompiler::compile(kParse->guard,
                                          kWs.elems,
                                          ElementType::BOOL,
                                          guardAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            // Override error text set by expression compiler for consistent
            // state machine compiler error messages.
            if (kErr != nullptr)
            {
                kErr->text = gErrText;
            }

            return res;
        }

        // Add compiled expression to workspace.
        kWs.exprAsms.push_back(guardAsm);

        // Put compiled guard raw pointer in owning block.
        SF_SAFE_ASSERT(guardAsm != nullptr);
        SF_SAFE_ASSERT(guardAsm->root() != nullptr);
        SF_SAFE_ASSERT(guardAsm->root()->type() == ElementType::BOOL);
        kBlock->guard = dynamic_cast<IExprNode<bool>*>(guardAsm->root().get());

        if (kParse->ifBlock != nullptr)
        {
            // Compile if branch block.
            Ref<StateMachine::Block> block;
            res = StateMachineCompiler::compileBlock(kParse->ifBlock,
                                                     kWs,
                                                     kInExitLabel,
                                                     block,
                                                     kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Put compiled block raw pointer in owning block.
            SF_SAFE_ASSERT(block != nullptr);
            kBlock->ifBlock = block.get();
        }

        if (kParse->elseBlock != nullptr)
        {
            // Compile else branch block.
            Ref<StateMachine::Block> block;
            res = StateMachineCompiler::compileBlock(kParse->elseBlock,
                                                     kWs,
                                                     kInExitLabel,
                                                     block,
                                                     kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Put compiled block raw pointer in owning block.
            SF_SAFE_ASSERT(block != nullptr);
            kBlock->elseBlock = block.get();
        }
    }

    if (kParse->action != nullptr)
    {
        // Compile action.
        Ref<IAction> action;
        res = StateMachineCompiler::compileAction(kParse->action,
                                                  kWs,
                                                  kInExitLabel,
                                                  action,
                                                  kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled action raw pointer in block struct.
        SF_SAFE_ASSERT(action != nullptr);
        kBlock->action = action.get();
    }

    if (kParse->next != nullptr)
    {
        // Compile next block.
        Ref<StateMachine::Block> block;
        res = StateMachineCompiler::compileBlock(kParse->next,
                                                 kWs,
                                                 kInExitLabel,
                                                 block,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning block.
        SF_SAFE_ASSERT(block != nullptr);
        kBlock->next = block.get();
    }

    return SUCCESS;
}

Result StateMachineCompiler::compileState(
    const StateMachineParse::StateParse& kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kWs.stateConfigs != nullptr);

    // Check that state name is not reserved.
    if (StateMachineCompiler::stateNameReserved(kParse.tokName))
    {
        ErrorInfo::set(kErr, kParse.tokName, gErrText,
                       "state name is reserved");
        return E_SMC_RSVD;
    }

    // State ID is the current number of compiled states + 1 so that state IDs
    // begin at 1.
    StateMachine::StateConfig stateConfig =
    {
        static_cast<U32>(kWs.stateConfigs->size() + 1),
        nullptr,
        nullptr,
        nullptr
    };

    Result res = SUCCESS;
    if (kParse.entry != nullptr)
    {
        // Compile entry label.
        Ref<StateMachine::Block> entryBlock;
        res = StateMachineCompiler::compileBlock(kParse.entry,
                                                 kWs,
                                                 false,
                                                 entryBlock,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning state.
        SF_SAFE_ASSERT(entryBlock != nullptr);
        stateConfig.entry = entryBlock.get();
    }

    if (kParse.step != nullptr)
    {
        // Compile step label.
        Ref<StateMachine::Block> stepBlock;
        res = StateMachineCompiler::compileBlock(kParse.step,
                                                 kWs,
                                                 false,
                                                 stepBlock,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning state.
        SF_SAFE_ASSERT(stepBlock != nullptr);
        stateConfig.step = stepBlock.get();
    }

    if (kParse.exit != nullptr)
    {
        // Compile exit label.
        Ref<StateMachine::Block> exitBlock;
        res = StateMachineCompiler::compileBlock(kParse.exit,
                                                 kWs,
                                                 true,
                                                 exitBlock,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning state.
        SF_SAFE_ASSERT(exitBlock != nullptr);
        stateConfig.exit = exitBlock.get();
    }

    // Add state config to workspace.
    kWs.stateConfigs->push_back(stateConfig);

    return SUCCESS;
}

StateMachineAssembly::StateMachineAssembly(
    const StateMachineAssembly::Workspace& kWs) : mWs(kWs)
{
}

} // namespace Sf
