#include <fstream>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionAssembly.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineAssembly.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state machine config error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateMachineAssembly::compile(
    const String kFilePath,
    const Ref<const StateVectorAssembly> kSvAsm,
    Ref<const StateMachineAssembly>& kAsm,
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
    return StateMachineAssembly::compile(ifs, kSvAsm, kAsm, kErr);
}

Result StateMachineAssembly::compile(
    std::istream& kIs,
    const Ref<const StateVectorAssembly> kSvAsm,
    Ref<const StateMachineAssembly>& kAsm,
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

    // Parse the state machine config.
    Ref<const StateMachineParse> parse;
    res = StateMachineParse::parse(toks, parse, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }
        return res;
    }

    // Send state machine config into the next compilation phase.
    return StateMachineAssembly::compile(parse, kSvAsm, kAsm, kErr);
}

Result StateMachineAssembly::compile(
    const Ref<const StateMachineParse> kParse,
    const Ref<const StateVectorAssembly> kSvAsm,
    Ref<const StateMachineAssembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Check that state machine parse is non-null.
    if (kParse == nullptr)
    {
        return E_SMA_NULL;
    }

    // Initialize a blank workspace for the compilation.
    StateMachineAssembly::Workspace ws = {};

    // Put the state machine parse in the workspace so that it can be recalled
    // later.
    ws.smParse = kParse;

    // Put the state vector assembly in the workspace so that it can be recalled
    // later.
    ws.svAsm = kSvAsm;

    // Allocate state machine state config array.
    ws.stateConfigs.reset(new Vec<StateMachine::StateConfig>());

    // Validate the state machine state vector. This will partially populate
    // the element symbol table in the compiler state.
    Result res = StateMachineAssembly::checkStateVector(kParse, ws, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile the local state vector. This will complete the element symbol
    // table in the compiler state.
    res = StateMachineAssembly::compileLocalStateVector(kParse, ws, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set local element initial values.
    res = StateMachineAssembly::initLocalElementValues(kParse, ws, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Build map of state names to IDs. IDs begin at 1 and count up in the order
    // states are defined in the config.
    for (U32 i = 0; i < kParse->states.size(); ++i)
    {
        const String& tokNameStr = kParse->states[i].tokName.str;
        const String stateName = tokNameStr.substr(1, (tokNameStr.size() - 2));
        ws.stateIds[stateName] = (i + 1);
    }

    // Compile each state machine state.
    for (const StateMachineParse::StateParse& state : kParse->states)
    {
        res = StateMachineAssembly::compileState(state, ws, kErr);
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

    // Config is done- create new state machine with it. The config is given the
    // raw pointers underlying the previously allocated state config and
    // expression stats vectors, as well as raw pointers of certain state vector
    // elements.
    SF_SAFE_ASSERT(ws.elems[LangConst::elemNameState] != nullptr);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemNameStateTime] != nullptr);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemNameGlobalTime] != nullptr);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemNameState]->type()
                   == ElementType::UINT32);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemNameStateTime]->type()
                   == ElementType::UINT64);
    SF_SAFE_ASSERT(ws.elems[LangConst::elemNameGlobalTime]->type()
                   == ElementType::UINT64);
    const StateMachine::Config smConfig =
    {
        static_cast<Element<U32>*>(ws.elems[LangConst::elemNameState]),
        static_cast<Element<U64>*>(ws.elems[LangConst::elemNameStateTime]),
        static_cast<Element<U64>*>(ws.elems[LangConst::elemNameGlobalTime]),
        ws.stateConfigs->data(),
        ws.exprStatArr->data()
    };
    ws.sm.reset(new StateMachine());
    res = StateMachine::create(smConfig, *ws.sm);
    if (res != SUCCESS)
    {
        // Since the state machine config is known correct, the most likely
        // error here is that the user didn't set a valid initial state in the
        // state element.
        return res;
    }

    // Compilation is done- clear the workspace of unneeded data before we pass
    // it to the final assembly.
    ws.elems.clear();
    ws.stateIds.clear();
    ws.readOnlyElems.clear();

    // Create the final assembly.
    kAsm.reset(new StateMachineAssembly(ws));

    return SUCCESS;
}

Ref<StateMachine> StateMachineAssembly::get() const
{
    return mWs.sm;
}

StateMachine::Config StateMachineAssembly::config() const
{
    return mWs.smConfig;
}

Ref<const StateMachineParse> StateMachineAssembly::parse() const
{
    return mWs.smParse;
}

Ref<StateVector> StateMachineAssembly::localStateVector() const
{
    return mWs.localSvAsm->get();
}

/////////////////////////////////// Private ////////////////////////////////////

Result StateMachineAssembly::checkStateVector(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);
    SF_SAFE_ASSERT(kWs.svAsm != nullptr);
    SF_SAFE_ASSERT(kWs.svAsm->get() != nullptr);

    for (const StateMachineParse::StateVectorElementParse& elem :
         kParse->svElems)
    {
        // Get element object from state vector.
        IElement* elemObj = nullptr;
        const Ref<StateVector> sv = kWs.svAsm->get();
        SF_SAFE_ASSERT(sv != nullptr);
        if (sv->getIElement(elem.tokName.str.c_str(), elemObj) != SUCCESS)
        {
            // Element does not exist in state vector.
            ConfigUtil::setError(kErr,
                                 elem.tokName,
                                 gErrText,
                                 ("element `" + elem.tokName.str
                                  + "` does not exist in state vector"));
            return E_SMA_SV_ELEM;
        }
        SF_SAFE_ASSERT(elemObj != nullptr);

        // Look up element type as configured in the state machine.
        auto smTypeInfoIt = TypeInfo::fromName.find(elem.tokType.str);
        if (smTypeInfoIt == TypeInfo::fromName.end())
        {
            // Unknown type.
            ConfigUtil::setError(kErr,
                                 elem.tokType,
                                 gErrText,
                                 ("unknown type `" + elem.tokType.str + "`"));
            return E_SMA_TYPE;
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
            ConfigUtil::setError(kErr, elem.tokType, gErrText, ss.str());
            return E_SMA_TYPE_MISM;
        }

        // Check that element does not appear twice in the state machine.
        if (kWs.elems.find(elem.tokName.str) != kWs.elems.end())
        {
            ConfigUtil::setError(kErr, elem.tokName, gErrText,
                                 "element `" + elem.tokName.str + "` is listed "
                                 "more than once");
            return E_SMA_ELEM_DUPE;
        }

        // Add element to the symbol table.
        kWs.elems[elem.tokName.str] = elemObj;

        // Make a copy of the element read-only flag. The read-onlyness may
        // change in special cases.
        bool elemReadOnly = elem.readOnly;

        // Check for global time element.
        if ((elem.tokName.str == LangConst::elemNameGlobalTime)
            || (elem.alias == LangConst::elemNameGlobalTime))
        {
            // Global time element is implicitly read-only.
            elemReadOnly = true;

            // Check that global time element is U64.
            if (smTypeInfo.enumVal != ElementType::UINT64)
            {
                std::stringstream ss;
                ss << "`" << LangConst::elemNameGlobalTime
                   << "` must be type U64 (" << elem.tokType.str << " here)";
                ConfigUtil::setError(kErr, elem.tokName, gErrText, ss.str());
                return E_SMA_G_TYPE;
            }
        }

        // Check for state element.
        if ((elem.tokName.str == LangConst::elemNameState)
            || (elem.alias == LangConst::elemNameState))
        {
            // State element is implicitly read-only.
            elemReadOnly = true;

            // Check that state element is U32.
            if (smTypeInfo.enumVal != ElementType::UINT32)
            {
                std::stringstream ss;
                ss << "`" << LangConst::elemNameState << "` must be type U32 ("
                   << elem.tokType.str << " here)";
                ConfigUtil::setError(kErr, elem.tokName, gErrText, ss.str());
                return E_SMA_S_TYPE;
            }
        }

        // If the element is aliased, add the alias to the symbol table as well.
        if (elem.alias.size() > 0)
        {
            kWs.elems[elem.alias] = elemObj;
        }

        // If element is read-only, add its name and alias to read-only set.
        if (elemReadOnly)
        {
            kWs.readOnlyElems.insert(elem.tokName.str);
            if (elem.alias.size() > 0)
            {
                kWs.readOnlyElems.insert(elem.alias);
            }
        }
    }

    // Check that a global time element was provided.
    if (kWs.elems.find(LangConst::elemNameGlobalTime) == kWs.elems.end())
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            kErr->subtext = ("no global time element aliased to `"
                             + LangConst::elemNameGlobalTime + "`");
        }
        return E_SMA_NO_G;
    }

    // Check that a state element was provided.
    if (kWs.elems.find(LangConst::elemNameState) == kWs.elems.end())
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            kErr->subtext = ("no state element aliased to `"
                             + LangConst::elemNameState + "`");
        }
        return E_SMA_NO_S;
    }

    return SUCCESS;
}

Result StateMachineAssembly::compileLocalStateVector(
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
                  << "U64 " << LangConst::elemNameStateTime << "\n";

    // State time element is implicitly read-only.
    kWs.readOnlyElems.insert(LangConst::elemNameStateTime);

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
                || ((svElem.alias.size() > 0)
                    && (elem.tokName.str == svElem.alias)))
            {
                std::stringstream ss;
                ss << "reuse of element name `" << elem.tokName.str
                   << "` (previously used on line " << svElem.tokName.lineNum
                   << ")";
                ConfigUtil::setError(kErr, elem.tokName, gErrText, ss.str());
                return E_SMA_ELEM_DUPE;
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
    Result res = StateVectorAssembly::compile(localSvConfig,
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
        res = kWs.localSvAsm->get()->getIElement(elem.tokName.str.c_str(),
                                                 elemObj);
        SF_SAFE_ASSERT(res == SUCCESS);
        SF_SAFE_ASSERT(elemObj != nullptr);

        // Add element to symbol table.
        kWs.elems[elem.tokName.str] = elemObj;
    }

    return SUCCESS;
}

Result StateMachineAssembly::checkLocalElemInitExprs(
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
            ConfigUtil::setError(kErr,
                                 kExpr->data,
                                 gErrText,
                                 "cannot use element to initialize itself");
            return E_SMA_SELF_REF;
        }

        // Check that element is not a non-local state vector element.
        IElement* elemObj = nullptr;
        const Ref<StateVector> sv = kWs.svAsm->get();
        SF_SAFE_ASSERT(sv != nullptr);
        if (sv->getIElement(kExpr->data.str.c_str(), elemObj) == SUCCESS)
        {
            std::stringstream ss;
            ss << "illegal reference to non-local element `"
               << kExpr->data.type << "`";
            ConfigUtil::setError(kErr, kExpr->data, gErrText, ss.str());
            return E_SMA_LOC_SV_REF;
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
                ConfigUtil::setError(kErr, kExpr->data, gErrText, ss.str());
                return E_SMA_UBI;
            }
        }
    }

    // Check left subtree.
    Result res = StateMachineAssembly::checkLocalElemInitExprs(kInitElem,
                                                               kExpr->left,
                                                               kWs,
                                                               kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Check right subtree.
    res = StateMachineAssembly::checkLocalElemInitExprs(kInitElem,
                                                        kExpr->right,
                                                        kWs,
                                                        kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    return SUCCESS;
}

Result StateMachineAssembly::initLocalElementValues(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);
    SF_SAFE_ASSERT(kWs.localSvAsm != nullptr);
    SF_SAFE_ASSERT(kWs.localSvAsm->get() != nullptr);

    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        // Validate element references in the initialization expression.
        Result res = StateMachineAssembly::checkLocalElemInitExprs(
            elem,
            elem.initValExpr,
            kWs,
            kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Look up element object so that we can get its type as an enum.
        IElement* const elemObj = kWs.elems[elem.tokName.str];
        SF_SAFE_ASSERT(elemObj != nullptr);

        // Compile element initial value expression.
        Ref<const ExpressionAssembly> initExprAsm;
        res = ExpressionAssembly::compile(elem.initValExpr,
                                          kWs.elems,
                                          elemObj->type(),
                                          initExprAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Evaluate expression and write to element. The element and expression
        // pointers are narrowed to template instantiations matching the
        // element's type. These casts are guaranteed valid by the element and
        // expression implementations.
        SF_SAFE_ASSERT(initExprAsm != nullptr);
        IExpression* const iroot = initExprAsm->root().get();
        SF_SAFE_ASSERT(iroot != nullptr);
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT8);
                Element<I8>* const elem = static_cast<Element<I8>*>(elemObj);
                IExprNode<I8>* const root = static_cast<IExprNode<I8>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::INT16:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT16);
                Element<I16>* const elem = static_cast<Element<I16>*>(elemObj);
                IExprNode<I16>* const root =
                    static_cast<IExprNode<I16>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::INT32:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT32);
                Element<I32>* const elem = static_cast<Element<I32>*>(elemObj);
                IExprNode<I32>* const root =
                    static_cast<IExprNode<I32>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::INT64:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::INT64);
                Element<I64>* const elem = static_cast<Element<I64>*>(elemObj);
                IExprNode<I64>* const root =
                    static_cast<IExprNode<I64>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT8:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT8);
                Element<U8>* const elem = static_cast<Element<U8>*>(elemObj);
                IExprNode<U8>* const root = static_cast<IExprNode<U8>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT16:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT16);
                Element<U16>* const elem = static_cast<Element<U16>*>(elemObj);
                IExprNode<U16>* const root =
                    static_cast<IExprNode<U16>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT32:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT32);
                Element<U32>* const elem = static_cast<Element<U32>*>(elemObj);
                IExprNode<U32>* const root =
                    static_cast<IExprNode<U32>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::UINT64:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::UINT64);
                Element<U64>* const elem = static_cast<Element<U64>*>(elemObj);
                IExprNode<U64>* const root =
                    static_cast<IExprNode<U64>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::FLOAT32:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::FLOAT32);
                Element<F32>* const elem = static_cast<Element<F32>*>(elemObj);
                IExprNode<F32>* const root =
                    static_cast<IExprNode<F32>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::FLOAT64:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::FLOAT64);
                Element<F64>* const elem = static_cast<Element<F64>*>(elemObj);
                IExprNode<F64>* const root =
                    static_cast<IExprNode<F64>*>(iroot);
                elem->write(root->evaluate());
                break;
            }

            case ElementType::BOOL:
            {
                SF_SAFE_ASSERT(iroot->type() == ElementType::BOOL);
                Element<bool>* const elem =
                    static_cast<Element<bool>*>(elemObj);
                IExprNode<bool>* const root =
                    static_cast<IExprNode<bool>*>(iroot);
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

Result StateMachineAssembly::compileAssignmentAction(
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
        ConfigUtil::setError(kErr,
                             kParse->tokLhs,
                             gErrText,
                             ("unknown element `" + kParse->tokLhs.str
                              + "`"));
        return E_SMA_ASG_ELEM;
    }
    IElement* const elemObj = (*elemIt).second;
    SF_SAFE_ASSERT(elemObj != nullptr);

    // Check that LHS element is not read-only.
    if (kReadOnlyElems.find(kParse->tokLhs.str) != kReadOnlyElems.end())
    {
        ConfigUtil::setError(kErr,
                             kParse->tokLhs,
                             gErrText,
                             ("element `" + kParse->tokLhs.str
                              + "` is read-only"));
        return E_SMA_ELEM_RO;
    }

    // Compile RHS expression.
    const Result res = ExpressionAssembly::compile(kParse->rhs,
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
    // and RHS root node are narrowed to template instantiations that match
    // the element type. These casts are guaranteed correct in this context
    // by the element and expression compiler implementations.
    SF_SAFE_ASSERT(kRhsAsm != nullptr);
    SF_SAFE_ASSERT(kRhsAsm->root() != nullptr);
    switch (elemObj->type())
    {
        case ElementType::INT8:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT8);
            kAction.reset(new AssignmentAction<I8>(
                *static_cast<Element<I8>*>(elemObj),
                *static_cast<IExprNode<I8>*>(kRhsAsm->root().get())));
            break;

        case ElementType::INT16:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT16);
            kAction.reset(new AssignmentAction<I16>(
                *static_cast<Element<I16>*>(elemObj),
                *static_cast<IExprNode<I16>*>(kRhsAsm->root().get())));
            break;

        case ElementType::INT32:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT32);
            kAction.reset(new AssignmentAction<I32>(
                *static_cast<Element<I32>*>(elemObj),
                *static_cast<IExprNode<I32>*>(kRhsAsm->root().get())));
            break;

        case ElementType::INT64:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::INT64);
            kAction.reset(new AssignmentAction<I64>(
                *static_cast<Element<I64>*>(elemObj),
                *static_cast<IExprNode<I64>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT8:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT8);
            kAction.reset(new AssignmentAction<U8>(
                *static_cast<Element<U8>*>(elemObj),
                *static_cast<IExprNode<U8>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT16:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT16);
            kAction.reset(new AssignmentAction<U16>(
                *static_cast<Element<U16>*>(elemObj),
                *static_cast<IExprNode<U16>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT32:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT32);
            kAction.reset(new AssignmentAction<U32>(
                *static_cast<Element<U32>*>(elemObj),
                *static_cast<IExprNode<U32>*>(kRhsAsm->root().get())));
            break;

        case ElementType::UINT64:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::UINT64);
            kAction.reset(new AssignmentAction<U64>(
                *static_cast<Element<U64>*>(elemObj),
                *static_cast<IExprNode<U64>*>(kRhsAsm->root().get())));
            break;

        case ElementType::FLOAT32:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::FLOAT32);
            kAction.reset(new AssignmentAction<F32>(
                *static_cast<Element<F32>*>(elemObj),
                *static_cast<IExprNode<F32>*>(kRhsAsm->root().get())));
            break;

        case ElementType::FLOAT64:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::FLOAT64);
            kAction.reset(new AssignmentAction<F64>(
                *static_cast<Element<F64>*>(elemObj),
                *static_cast<IExprNode<F64>*>(kRhsAsm->root().get())));
            break;

        case ElementType::BOOL:
            SF_SAFE_ASSERT(kRhsAsm->root()->type() == ElementType::BOOL);
            kAction.reset(new AssignmentAction<bool>(
                *static_cast<Element<bool>*>(elemObj),
                *static_cast<IExprNode<bool>*>(kRhsAsm->root().get())));
            break;

        default:
            // Unreachable.
            SF_SAFE_ASSERT(false);
    }

    return SUCCESS;
}

Result StateMachineAssembly::compileAction(
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
        res = StateMachineAssembly::compileAssignmentAction(kParse,
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
            ConfigUtil::setError(kErr,
                                 kParse->tokTransitionKeyword,
                                 gErrText,
                                 "illegal transition in exit label");
            return E_SMA_TR_EXIT;
        }

        // Validate destination state.
        auto stateIdIt = kWs.stateIds.find(kParse->tokDestState.str);
        if (stateIdIt == kWs.stateIds.end())
        {
            ConfigUtil::setError(kErr,
                                 kParse->tokDestState,
                                 gErrText,
                                 ("unknown state `" + kParse->tokDestState.str
                                  + "`"));
            return E_SMA_STATE;
        }

        // Create transition action with destination state.
        const U32 destState = (*stateIdIt).second;
        kAction.reset(new TransitionAction(destState));
    }

    // Add compiled action to the workspace.
    kWs.actions.push_back(kAction);

    return SUCCESS;
}

Result StateMachineAssembly::compileBlock(
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
        ConfigUtil::setError(kErr,
                             kParse->tokAssert,
                             gErrText,
                             ("`" + kParse->tokAssert.str + "` may only be used"
                              " in state scripts"));
        return E_SMA_ASSERT;
    }

    // Check that a stop annotation, which is only allowed in state scripts, is
    // not being used in the state machine.
    if (kParse->tokStop.str.size() > 0)
    {
        ConfigUtil::setError(kErr,
                             kParse->tokStop,
                             gErrText,
                             ("`" + kParse->tokStop.str + "` may only be used "
                              "in state scripts"));
        return E_SMA_STOP;
    }

    // Allocate new block and add to workspace.
    kBlock.reset(new StateMachine::Block{});
    kWs.blocks.push_back(kBlock);

    Result res = SUCCESS;
    if (kParse->guard != nullptr)
    {
        // Compile block guard.
        Ref<const ExpressionAssembly> guardAsm;
        res = ExpressionAssembly::compile(kParse->guard,
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
        kBlock->guard = static_cast<IExprNode<bool>*>(guardAsm->root().get());

        if (kParse->ifBlock != nullptr)
        {
            // Compile if branch block.
            Ref<StateMachine::Block> block;
            res = StateMachineAssembly::compileBlock(kParse->ifBlock,
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
            res = StateMachineAssembly::compileBlock(kParse->elseBlock,
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
        res = StateMachineAssembly::compileAction(kParse->action,
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
        res = StateMachineAssembly::compileBlock(kParse->next,
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

Result StateMachineAssembly::compileState(
    const StateMachineParse::StateParse& kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kWs.stateConfigs != nullptr);

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
        res = StateMachineAssembly::compileBlock(kParse.entry,
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
        res = StateMachineAssembly::compileBlock(kParse.step,
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
        res = StateMachineAssembly::compileBlock(kParse.exit,
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
