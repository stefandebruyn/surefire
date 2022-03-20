#include <fstream>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionAssembly.hpp"
#include "sf/config/StateMachineAssembly.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/core/Assert.hpp"

static const char* const errText = "state machine config error";

static const String elemStateTimeName = "T";

static const String elemGlobalTimeName = "G";

static const String elemStateName = "S";

/////////////////////////////////// Public /////////////////////////////////////

Result StateMachineAssembly::compile(const String kFilePath,
                                    const Ref<StateVector> kSv,
                                    Ref<const StateMachineAssembly>& kAsm,
                                    ErrorInfo* const kErr)
{
    // Set the error info file path so that error messages generated further
    // into compilation have the file path.
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

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

    // Send input stream into the next compilation phase.
    return StateMachineAssembly::compile(ifs, kSv, kAsm, kErr);
}

Result StateMachineAssembly::compile(std::istream& kIs,
                                     const Ref<StateVector> kSv,
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
            kErr->text = errText;
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
            kErr->text = errText;
        }
        return res;
    }

    // Send state machine config into the next compilation phase.
    return StateMachineAssembly::compile(parse, kSv, kAsm, kErr);
}

Result StateMachineAssembly::compile(const Ref<const StateMachineParse> kParse,
                                     const Ref<StateVector> kSv,
                                     Ref<const StateMachineAssembly>& kAsm,
                                     ErrorInfo* const kErr)
{
    // Initialize a blank workspace for the compilation.
    StateMachineAssembly::Workspace ws = {};

    // Allocate a vector for storing state machine configs.
    ws.stateConfigs.reset(new Vec<StateMachine::StateConfig>());

    // Validate the state machine state vector. This will partially populate
    // the element symbol table in the compiler state.
    Result res = StateMachineAssembly::checkStateVector(kParse, kSv, ws, kErr);
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
        res = StateMachineAssembly::compileState(state, kSv, ws, kErr);
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

    // Copy addresses of expression stats into a vector. The memory underlying
    // this vector will be directly provided to the state machine config. The
    // memory is twice-wrapped in a vector and then a shared pointer to
    // automatically handle deallocation. (C++11 shared pointers are not
    // specialized for native arrays, so a vector is used instead.)
    const Ref<Vec<IExpressionStats*>> exprStatArr(new Vec<IExpressionStats*>());
    for (const Ref<IExpressionStats> exprStats : allExprStats)
    {
        exprStatArr->push_back(exprStats.get());
    }

    // Add expression stats array null terminator required by state machine.
    exprStatArr->push_back(nullptr);

    // Add null terminator to state config vector required by state machine. The
    // memory underlying this vector will be directly provided to the state
    // machine config.
    ws.stateConfigs->push_back({StateMachine::NO_STATE,
                                nullptr,
                                nullptr,
                                nullptr});

    // Config is done- create new state machine with it. The config is given the
    // raw pointers of the state config and expression stats arrays underlying
    // the previously allocated vectors, as well as raw pointers of certain
    // state vector elements. The lifetimes of these elements are managed by
    // the global/local state vector assemblies.
    const StateMachine::Config smConfig =
    {
        static_cast<Element<U32>*>(ws.elems["S"]),
        static_cast<Element<U64>*>(ws.elems["T"]),
        static_cast<Element<U64>*>(ws.elems["G"]),
        ws.stateConfigs->data(),
        exprStatArr->data()
    };
    const Ref<StateMachine> sm(new StateMachine());
    res = StateMachine::create(smConfig, *sm);
    SF_ASSERT(res == SUCCESS);

    // Create the final assembly.
    kAsm.reset(new StateMachineAssembly(sm,
                                        smConfig,
                                        kParse,
                                        ws.localSvAsm,
                                        ws.exprAsms,
                                        ws.stateConfigs,
                                        exprStatArr,
                                        ws.blocks,
                                        ws.actions));

    return SUCCESS;
}

Ref<StateMachine> StateMachineAssembly::get() const
{
    return mObj;
}

StateMachine::Config StateMachineAssembly::config() const
{
    return mConfig;
}

Ref<const StateMachineParse> StateMachineAssembly::parse() const
{
    return mParse;
}

Ref<StateVector> StateMachineAssembly::localStateVector() const
{
    return mLocalSvAsm->get();
}

/////////////////////////////////// Private ////////////////////////////////////

Result StateMachineAssembly::checkStateVector(
    const Ref<const StateMachineParse> kParse,
    const Ref<StateVector> kSv,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_ASSERT(kParse != nullptr);
    SF_ASSERT(kSv != nullptr);

    for (const StateMachineParse::StateVectorElementParse& elem :
         kParse->svElems)
    {
        // Get element object from state vector.
        IElement* elemObj = nullptr;
        if (kSv->getIElement(elem.tokName.str.c_str(), elemObj) != SUCCESS)
        {
            // Element does not exist in state vector.
            ConfigUtil::setError(kErr, elem.tokName, errText,
                                 "element `" + elem.tokName.str + "` does not "
                                 "exist in state vector");
            return E_SMC_SV_ELEM;
        }
        SF_ASSERT(elemObj != nullptr);

        // Look up element type as configured in the state machine.
        auto smTypeInfoIt = ElementTypeInfo::fromName.find(elem.tokType.str);
        if (smTypeInfoIt == ElementTypeInfo::fromName.end())
        {
            // Unknown type.
            ConfigUtil::setError(kErr, elem.tokType, errText,
                                 "unknown type `" + elem.tokType.str + "`");
            return E_SMC_TYPE;
        }
        const ElementTypeInfo& smTypeInfo = (*smTypeInfoIt).second;

        // Look up element type info as configured in the actual state vector.
        auto typeInfoIt = ElementTypeInfo::fromEnum.find(elemObj->type());
        // Assert that lookup succeeds since valid element types are guaranteed
        // by the state vector.
        SF_ASSERT(typeInfoIt != ElementTypeInfo::fromEnum.end());
        const ElementTypeInfo& typeInfo = (*typeInfoIt).second;

        // Check that element has the same type in the state vector and state
        // machine.
        if (typeInfo.enumVal != smTypeInfo.enumVal)
        {
            std::stringstream ss;
            ss << "element `" << elem.tokName.str << "` is type "
               << typeInfo.name << " in the state vector but type "
               << smTypeInfo.name << " here";
            ConfigUtil::setError(kErr, elem.tokType, errText, ss.str());
            return E_SMC_TYPE_MISM;
        }

        // Check that element does not appear twice in the state machine.
        if (kWs.elems.find(elem.tokName.str) != kWs.elems.end())
        {
            ConfigUtil::setError(kErr, elem.tokName, errText,
                                 "element `" + elem.tokName.str + "` is listed "
                                 "more than once");
            return E_SMC_ELEM_DUPE;
        }

        // Add element to the symbol table.
        kWs.elems[elem.tokName.str] = elemObj;

        // Make a copy of the element read-only flag. The read-onlyness may
        // change in special cases.
        bool elemReadOnly = elem.readOnly;

        // Check that global time element is the correct type.
        if ((elem.tokName.str == elemGlobalTimeName)
            || (elem.alias == elemGlobalTimeName))
        {
            // Global time element is automatically read-only.
            elemReadOnly = true;

            if (smTypeInfo.enumVal != ElementType::UINT64)
            {
                // Global time element is not U64.
                std::stringstream ss;
                ss << "`" << elemGlobalTimeName << "` must be type U64 ("
                   << elem.tokType.str << " here)";
                ConfigUtil::setError(kErr, elem.tokName, errText, ss.str());
                return E_SMC_G_TYPE;
            }
        }

        // Check that state element is the correct type.
        if ((elem.tokName.str == elemStateName)
            || (elem.alias == elemStateName))
        {
            // State element is automatically read-only.
            elemReadOnly = true;

            if (smTypeInfo.enumVal != ElementType::UINT32)
            {
                // State element is not U32.
                std::stringstream ss;
                ss << "`" << elemStateName << "` must be type U32 ("
                   << elem.tokType.str << " here)";
                ConfigUtil::setError(kErr, elem.tokName, errText, ss.str());
                return E_SMC_S_TYPE;
            }
        }

        // If the element is aliased, add the alias to the symbol table too.
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

    if (kWs.elems.find(elemGlobalTimeName) == kWs.elems.end())
    {
        // No global time element provided.
        if (kErr != nullptr)
        {
            kErr->text = errText;
            kErr->subtext = "no global time element aliased to `"
                            + elemGlobalTimeName + "`";
        }
        return E_SMC_NO_G;
    }

    if (kWs.elems.find(elemStateName) == kWs.elems.end())
    {
        // No state element provided.
        if (kErr != nullptr)
        {
            kErr->text = errText;
            kErr->subtext = "no state element aliased to `" + elemStateName
                            + "`";
        }
        return E_SMC_NO_S;
    }

    return SUCCESS;
}

Result StateMachineAssembly::compileLocalStateVector(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    std::stringstream localSvConfig;
    localSvConfig << "[LOCAL]\n"
                  << "U64 " << elemStateTimeName << "\n";

    // State time element is automatically read-only.
    kWs.readOnlyElems.insert(elemStateTimeName);

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
                ConfigUtil::setError(kErr, elem.tokName, errText, ss.str());
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

    // Compile the local state vector. Since the local state vector parse is at
    // least syntatically correct, there are very few potential errors that the
    // state vector compiler can generate here.
    Result res = StateVectorAssembly::compile(localSvConfig,
                                              kWs.localSvAsm,
                                              kErr);
    SF_ASSERT(res == SUCCESS);

    // Look up each element object in the local state vector and add it to the
    // element symbol table.
    SF_ASSERT(kWs.localSvAsm != nullptr);
    SF_ASSERT(kWs.localSvAsm->parse() != nullptr);
    SF_ASSERT(kWs.localSvAsm->parse()->regions.size() == 1);
    for (const StateVectorParse::ElementParse& elem :
         kWs.localSvAsm->parse()->regions[0].elems)
    {
        IElement* elemObj = nullptr;
        res = kWs.localSvAsm->get()->getIElement(elem.tokName.str.c_str(),
                                                 elemObj);
        // Assert that element lookup succeeds since we configured the local
        // state vector in this function and know the element exists.
        SF_ASSERT(res == SUCCESS);
        SF_ASSERT(elemObj != nullptr);
        kWs.elems[elem.tokName.str] = elemObj;
    }

    return SUCCESS;
}

Result StateMachineAssembly::initLocalElementValues(
    const Ref<const StateMachineParse> kParse,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    (void) kParse;
    (void) kWs;
    (void) kErr; // rm later

    return SUCCESS;
}

Result StateMachineAssembly::compileAction(
    const Ref<const StateMachineParse::ActionParse> kParse,
    Ref<StateVector> kSv,
    StateMachineAssembly::Workspace& kWs,
    const bool kInExitLabel,
    Ref<IAction>& kAction,
    ErrorInfo* const kErr)
{
    Result res = SUCCESS;

    if (kParse->lhs != nullptr)
    {
        // Compile assignment action.

        // Look up RHS element.
        auto elemIt = kWs.elems.find(kParse->tokRhs.str);
        if (elemIt == kWs.elems.end())
        {
            // Unknown element.
            ConfigUtil::setError(
                kErr, kParse->tokRhs, errText,
                "unknown element `" + kParse->tokRhs.str + "`");
            return E_SMC_ASG_ELEM;
        }
        IElement* const elemObj = (*elemIt).second;

        // Check that RHS element is not read-only. This includes elements
        // marked read-only by the user and reserved elements.
        if ((kWs.readOnlyElems.find(kParse->tokRhs.str) !=
             kWs.readOnlyElems.end())
            || (kParse->tokRhs.str == elemGlobalTimeName)
            || (kParse->tokRhs.str == elemStateTimeName)
            || (kParse->tokRhs.str == elemStateName))
        {
            ConfigUtil::setError(kErr, kParse->tokRhs, errText,
                                 "element `" + kParse->tokRhs.str + "` is "
                                 "read-only");
            return E_SMC_ELEM_RO;
        }

        // Compile LHS expression.
        Ref<const ExpressionAssembly> lhsAsm;
        res = ExpressionAssembly::compile(kParse->lhs,
                                          {kSv, kWs.localSvAsm->get()},
                                          elemObj->type(),
                                          lhsAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            // Override error text set by expression compiler for consistent
            // state machine compiler error messages.
            if (kErr != nullptr)
            {
                kErr->text = errText;
            }

            return res;
        }

        // Add compiled expression to the workspace.
        kWs.exprAsms.push_back(lhsAsm);

        // Create assignment action based on element type. The element object
        // and LHS root nodes are narrowed to template instantiations that match
        // the element type. These casts are guaranteed correct in this context
        // by the element and expression compiler implementations.
        switch (elemObj->type())
        {
            case ElementType::INT8:
                kAction.reset(new AssignmentAction<I8>(
                    *static_cast<Element<I8>*>(elemObj),
                    *static_cast<IExprNode<I8>*>(lhsAsm->root().get())));
                break;

            case ElementType::INT16:
                kAction.reset(new AssignmentAction<I16>(
                    *static_cast<Element<I16>*>(elemObj),
                    *static_cast<IExprNode<I16>*>(lhsAsm->root().get())));
                break;

            case ElementType::INT32:
                kAction.reset(new AssignmentAction<I32>(
                    *static_cast<Element<I32>*>(elemObj),
                    *static_cast<IExprNode<I32>*>(lhsAsm->root().get())));
                break;

            case ElementType::INT64:
                kAction.reset(new AssignmentAction<I64>(
                    *static_cast<Element<I64>*>(elemObj),
                    *static_cast<IExprNode<I64>*>(lhsAsm->root().get())));
                break;

            case ElementType::UINT8:
                kAction.reset(new AssignmentAction<U8>(
                    *static_cast<Element<U8>*>(elemObj),
                    *static_cast<IExprNode<U8>*>(lhsAsm->root().get())));
                break;

            case ElementType::UINT16:
                kAction.reset(new AssignmentAction<U16>(
                    *static_cast<Element<U16>*>(elemObj),
                    *static_cast<IExprNode<U16>*>(lhsAsm->root().get())));
                break;

            case ElementType::UINT32:
                kAction.reset(new AssignmentAction<U32>(
                    *static_cast<Element<U32>*>(elemObj),
                    *static_cast<IExprNode<U32>*>(lhsAsm->root().get())));
                break;

            case ElementType::UINT64:
                kAction.reset(new AssignmentAction<U64>(
                    *static_cast<Element<U64>*>(elemObj),
                    *static_cast<IExprNode<U64>*>(lhsAsm->root().get())));
                break;

            case ElementType::FLOAT32:
                kAction.reset(new AssignmentAction<F32>(
                    *static_cast<Element<F32>*>(elemObj),
                    *static_cast<IExprNode<F32>*>(lhsAsm->root().get())));
                break;

            case ElementType::FLOAT64:
                kAction.reset(new AssignmentAction<F64>(
                    *static_cast<Element<F64>*>(elemObj),
                    *static_cast<IExprNode<F64>*>(lhsAsm->root().get())));
                break;

            case ElementType::BOOL:
                kAction.reset(new AssignmentAction<bool>(
                    *static_cast<Element<bool>*>(elemObj),
                    *static_cast<IExprNode<bool>*>(lhsAsm->root().get())));
                break;

            default:
                SF_ASSERT(false);
        }
    }
    else
    {
        // Compile transition action.

        if (kInExitLabel)
        {
            // Illegal transition in exit label.
            ConfigUtil::setError(kErr, kParse->tokDestState, errText,
                                 "illegal transition in exit label");
            return E_SMC_TR_EXIT;
        }

        auto stateIdIt = kWs.stateIds.find(kParse->tokDestState.str);
        if (stateIdIt == kWs.stateIds.end())
        {
            // Unknown destination state.
            ConfigUtil::setError(kErr, kParse->tokDestState, errText,
                                 "unknown state `" + kParse->tokDestState.str
                                 + "`");
            return E_SMC_STATE;
        }
        const U32 destState = (*stateIdIt).second;
        kAction.reset(new TransitionAction(destState));
    }

    // Add compiled action to the workspace.
    SF_ASSERT(kAction != nullptr);
    kWs.actions.push_back(kAction);

    return SUCCESS;
}

Result StateMachineAssembly::compileBlock(
    const Ref<const StateMachineParse::BlockParse> kParse,
    const Ref<StateVector> kSv,
    StateMachineAssembly::Workspace& kWs,
    const bool kInExitLabel,
    Ref<StateMachine::Block>& kBlock,
    ErrorInfo* const kErr)
{
    SF_ASSERT(kParse != nullptr);
    SF_ASSERT(kSv != nullptr);

    // Assertions are only allowed in state scripts, not state machines.
    if (kParse->assertion != nullptr)
    {
        // Error message will point to the first token in the assertion
        // expression, or the leftmost leaf in the tree.
        Ref<const ExpressionParse> node = kParse->assertion;
        while (node->left != nullptr)
        {
            node = node->left;
        }

        ConfigUtil::setError(kErr,
                             node->data,
                             errText,
                             "state machines may not contain assertions");
        return E_SMC_ASSERT;
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
                                          {kSv, kWs.localSvAsm->get()},
                                          ElementType::BOOL,
                                          guardAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            // Override error text set by expression compiler for consistent
            // state machine compiler error messages.
            if (kErr != nullptr)
            {
                kErr->text = errText;
            }

            return res;
        }

        // Add compiled expression to workspace.
        kWs.exprAsms.push_back(guardAsm);

        // Put compiled guard raw pointer in owning block.
        kBlock->guard = static_cast<IExprNode<bool>*>(guardAsm->root().get());

        if (kParse->ifBlock != nullptr)
        {
            // Compile if branch block.
            Ref<StateMachine::Block> block;
            res = StateMachineAssembly::compileBlock(kParse->ifBlock,
                                                     kSv,
                                                     kWs,
                                                     kInExitLabel,
                                                     block,
                                                     kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Put compiled block raw pointer in owning block.
            kBlock->ifBlock = block.get();
        }

        if (kParse->elseBlock != nullptr)
        {
            // Compile else branch block.
            Ref<StateMachine::Block> block;
            res = compileBlock(kParse->elseBlock,
                               kSv,
                               kWs,
                               kInExitLabel,
                               block,
                               kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Put compiled block raw pointer in owning block.
            kBlock->elseBlock = block.get();
        }
    }

    if (kParse->action != nullptr)
    {
        // Compile action.
        Ref<IAction> action;
        res = StateMachineAssembly::compileAction(kParse->action,
                                                  kSv,
                                                  kWs,
                                                  kInExitLabel,
                                                  action,
                                                  kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled action raw pointer in block struct.
        kBlock->action = action.get();
    }

    if (kParse->next != nullptr)
    {
        // Compile next block.
        Ref<StateMachine::Block> block;
        res = StateMachineAssembly::compileBlock(kParse->next,
                                                 kSv,
                                                 kWs,
                                                 kInExitLabel,
                                                 block,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning block.
        kBlock->next = block.get();
    }

    return SUCCESS;
}

Result StateMachineAssembly::compileState(
    const StateMachineParse::StateParse& kParse,
    Ref<StateVector> kSv,
    StateMachineAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_ASSERT(kWs.stateConfigs != nullptr);

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
                                                 kSv,
                                                 kWs,
                                                 false,
                                                 entryBlock,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning state.
        stateConfig.entry = entryBlock.get();
    }

    if (kParse.step != nullptr)
    {
        // Compile step label.
        Ref<StateMachine::Block> stepBlock;
        res = StateMachineAssembly::compileBlock(kParse.step,
                                                 kSv,
                                                 kWs,
                                                 false,
                                                 stepBlock,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning state.
        stateConfig.step = stepBlock.get();
    }

    if (kParse.exit != nullptr)
    {
        // Compile exit label.
        Ref<StateMachine::Block> exitBlock;
        res = StateMachineAssembly::compileBlock(kParse.exit,
                                                 kSv,
                                                 kWs,
                                                 true,
                                                 exitBlock,
                                                 kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Put compiled block raw pointer in owning state.
        stateConfig.exit = exitBlock.get();
    }

    // Add state config to workspace.
    kWs.stateConfigs->push_back(stateConfig);

    return SUCCESS;
}

StateMachineAssembly::StateMachineAssembly(
    const Ref<StateMachine> kObj,
    const StateMachine::Config kConfig,
    const Ref<const StateMachineParse> kParse,
    const Ref<const StateVectorAssembly> kLocalSvAsm,
    const Vec<Ref<const ExpressionAssembly>>& kExprAsms,
    const Ref<Vec<StateMachine::StateConfig>> kStateConfigs,
    const Ref<Vec<IExpressionStats*>> kExprStats,
    const Vec<Ref<StateMachine::Block>>& kBlocks,
    const Vec<Ref<IAction>>& kActions) :
    mObj(kObj),
    mConfig(kConfig),
    mParse(kParse),
    mLocalSvAsm(kLocalSvAsm),
    mExprAsms(kExprAsms),
    mStateConfigs(kStateConfigs),
    mExprStats(kExprStats),
    mBlocks(kBlocks),
    mActions(kActions)
{
}
