#include "sf/config/Autocode.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/core/Assert.hpp"
#include "sf/core/Expression.hpp"

/////////////////////////////////// Public /////////////////////////////////////

Result StateMachineAutocoder::code(std::ostream& kOs,
                                   const String kName,
                                   const Ref<const StateMachineAssembly> kSmAsm)
{
    // Check that parse and state vector assembly are non-null.
    if (kSmAsm == nullptr)
    {
        SF_ASSERT(false);
    }

    // Initialize a blank workspace for the autocoder.
    StateMachineAutocoder::Workspace ws{};
    ws.smAsm = kSmAsm;

    // Build map of state names to IDs.
    const Ref<const StateMachineParse> parse = kSmAsm->parse();
    SF_ASSERT(parse != nullptr);
    for (std::size_t i = 0; i < parse->states.size(); ++i)
    {
        const StateMachineParse::StateParse& state = parse->states[i];
        const String& sectionName = state.tokName.str;
        SF_ASSERT(sectionName.size() >= 3);
        const String stateName =
            sectionName.substr(1, (sectionName.size() - 2));
        ws.stateIds[stateName] = (i + 1);
    }

    // Add preamble.
    Autocode a(kOs);
    a("///");
    a("/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.");
    a("///");
    a();

    // Begin define guard.
    a("#ifndef %%_HPP", kName);
    a("#define %%_HPP", kName);
    a();

    // Add includes.
    a("#include \"sf/core/StateMachine.hpp\"");
    a("#include \"sf/core/StateVector.hpp\"");
    a();

    // Begin namespace.
    a("namespace %%", kName);
    a("{");
    a();

    // Add function signature.
    a("static Result getConfig(StateVector& kSv, StateMachine::Config& kSmConfig)");
    a("{");
    a.increaseIndent();

    a("Result res = SUCCESS;");
    a();

    // Generate code for local state vector.
    StateMachineAutocoder::codeLocalStateVector(a, ws);

    // Generate code for state configs.
    const StateMachine::Config smConfig = kSmAsm->config();
    for (const StateMachine::StateConfig* state = smConfig.states;
         state->id != StateMachine::NO_STATE;
         ++state)
    {
        StateMachineAutocoder::codeState(state, a, ws);
    }

    // Define state config array.
    a("// State machine config");
    a("static StateMachine::StateConfig stateConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const StateMachine::StateConfig* state = smConfig.states;
         state->id != StateMachine::NO_STATE;
         ++state)
    {
        a("state%%Config,", state->id);
    }

    a("{StateMachine::NO_STATE, nullptr, nullptr, nullptr}");
    a.decreaseIndent();
    a("};");
    a();

    // Define expression stats array.
    String exprStatsArrAddr = "nullptr";
    if (ws.statsCnt > 0)
    {
        a("static IExpressionStats* exprStats[] =");
        a("{");
        a.increaseIndent();

        for (U32 i = 0; i < ws.statsCnt; ++i)
        {
            a("&stats%%,", i);
        }

        a("nullptr"); // Null terminator
        a.decreaseIndent();
        a("};");
        a();

        exprStatsArrAddr = "exprStats";
    }

    // Generate code to look up state and global time element if not already.
    const String elemStateName = elemNameFromAddr(smConfig.elemState, ws);
    codeElementLookup(a, smConfig.elemState, TypeInfo::u32, elemStateName, ws);

    const String elemGlobalTimeName = elemNameFromAddr(smConfig.elemGlobalTime,
                                                       ws);
    codeElementLookup(a,
                      smConfig.elemGlobalTime,
                      TypeInfo::u64,
                      elemGlobalTimeName,
                      ws);
    a();

    // Define state machine config and return to caller.
    a("static StateMachine::Config smConfig = {elem%%, elem%%, elem%%, stateConfigs, %%};",
      elemStateName,
      LangConst::elemStateTime,
      elemGlobalTimeName,
      exprStatsArrAddr);
    a("kSmConfig = smConfig;");
    a();

    // Add return statement.
    a("return SUCCESS;");

    // Close function definition.
    a.decreaseIndent();
    a("}");
    a();

    // End namespace.
    a("} // namespace %%", kName);
    a();

    // End define guard.
    a("#endif");

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

const Map<const void*, String> StateMachineAutocoder::opFuncIds =
{
    // Binary operators
    {reinterpret_cast<void*>(&add<F64>), "add<F64>"},
    {reinterpret_cast<void*>(&sub<F64>), "sub<F64>"},
    {reinterpret_cast<void*>(&mult<F64>), "mult<F64>"},
    {reinterpret_cast<void*>(&div<F64>), "div<F64>"},
    {reinterpret_cast<void*>(&lt<F64>), "lt<F64>"},
    {reinterpret_cast<void*>(&lte<F64>), "lte<F64>"},
    {reinterpret_cast<void*>(&gt<F64>), "gt<F64>"},
    {reinterpret_cast<void*>(&gte<F64>), "gte<F64>"},
    {reinterpret_cast<void*>(&eq<F64>), "eq<F64>"},
    {reinterpret_cast<void*>(&neq<F64>), "neq<F64>"},
    {reinterpret_cast<void*>(&land<F64>), "land<F64>"},
    {reinterpret_cast<void*>(&lor<F64>), "lor<F64>"},
    // Unary operators
    {reinterpret_cast<void*>(&lnot<F64>), "lnot<F64>"},
    // Cast to F64
    {reinterpret_cast<void*>(&safeCast<F64, I8>), "safeCast<F64, I8>"},
    {reinterpret_cast<void*>(&safeCast<F64, I16>), "safeCast<F64, I16>"},
    {reinterpret_cast<void*>(&safeCast<F64, I32>), "safeCast<F64, I32>"},
    {reinterpret_cast<void*>(&safeCast<F64, I64>), "safeCast<F64, I64>"},
    {reinterpret_cast<void*>(&safeCast<F64, U8>), "safeCast<F64, U8>"},
    {reinterpret_cast<void*>(&safeCast<F64, U16>), "safeCast<F64, U16>"},
    {reinterpret_cast<void*>(&safeCast<F64, U32>), "safeCast<F64, U32>"},
    {reinterpret_cast<void*>(&safeCast<F64, U64>), "safeCast<F64, U64>"},
    {reinterpret_cast<void*>(&safeCast<F64, F32>), "safeCast<F64, F32>"},
    {reinterpret_cast<void*>(&safeCast<F64, F64>), "safeCast<F64, F64>"},
    {reinterpret_cast<void*>(&safeCast<F64, bool>), "safeCast<F64, bool>"},
    // Cast from F64
    {reinterpret_cast<void*>(&safeCast<I8, F64>), "safeCast<I8, F64>"},
    {reinterpret_cast<void*>(&safeCast<I16, F64>), "safeCast<I16, F64>"},
    {reinterpret_cast<void*>(&safeCast<I32, F64>), "safeCast<I32, F64>"},
    {reinterpret_cast<void*>(&safeCast<I64, F64>), "safeCast<I64, F64>"},
    {reinterpret_cast<void*>(&safeCast<U8, F64>), "safeCast<U8, F64>"},
    {reinterpret_cast<void*>(&safeCast<U16, F64>), "safeCast<U16, F64>"},
    {reinterpret_cast<void*>(&safeCast<U32, F64>), "safeCast<U32, F64>"},
    {reinterpret_cast<void*>(&safeCast<U64, F64>), "safeCast<U64, F64>"},
    {reinterpret_cast<void*>(&safeCast<F32, F64>), "safeCast<F32, F64>"},
    {reinterpret_cast<void*>(&safeCast<F64, F64>), "safeCast<F64, F64>"},
    {reinterpret_cast<void*>(&safeCast<bool, F64>), "safeCast<bool, F64>"}
};

const Map<IExpression::NodeType, String>
    StateMachineAutocoder::exprStatNodeIds =
{
    {IExpression::ROLL_AVG, "RollAvgNode"},
    {IExpression::ROLL_MEDIAN, "RollMedianNode"},
    {IExpression::ROLL_MIN, "RollMinNode"},
    {IExpression::ROLL_MAX, "RollMaxNode"},
    {IExpression::ROLL_RANGE, "RollRangeNode"}
};

String StateMachineAutocoder::elemNameFromAddr(
    const IElement* const kAddr,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kWs.smAsm != nullptr);

    // Grab pointers to global and local state vector assemblies.
    const Ref<const StateVectorAssembly> svAsm = kWs.smAsm->mWs.svAsm;
    const Ref<const StateVectorAssembly> localSvAsm = kWs.smAsm->mWs.localSvAsm;

    SF_ASSERT(svAsm != nullptr);
    SF_ASSERT(localSvAsm != nullptr);

    // Element pointer will be looked up in the configs for each state vector.
    const StateVector::Config svConfigs[2] =
        {svAsm->config(), localSvAsm->config()};

    for (std::size_t i = 0; i < sizeof(svConfigs); ++i)
    {
        for (const StateVector::ElementConfig* elem = svConfigs[i].elems;
             elem->name != nullptr;
             ++elem)
        {
            if (elem->elem == kAddr)
            {
                return elem->name;
            }
        }
    }

    // If we got this far, the element name was not found. This should be
    // unreachable since state machine compilation has already validated all
    // the element references that appear in expressions.
    SF_ASSERT(false);
    return "(unknown element)";
}

void StateMachineAutocoder::codeLocalStateVector(
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    Autocode& a = kAutocode;

    // Get local state vector config.
    const Ref<const StateVectorAssembly> localSvAsm = kWs.smAsm->mWs.localSvAsm;
    SF_ASSERT(localSvAsm != nullptr);
    const StateVector::Config localSvConfig = localSvAsm->config();

    // Define backing storage struct.
    a("// Local state vector");
    a("static struct");
    a("{");
    a.increaseIndent();

    Vec<String> elemDefs;

    // Loop over local elements.
    for (const StateVector::ElementConfig* elem = localSvConfig.elems;
         elem->name != nullptr;
         ++elem)
    {
        // Look up element type info.
        const IElement* const elemObj = elem->elem;
        SF_ASSERT(elemObj != nullptr);
        auto typeInfoIt = TypeInfo::fromEnum.find(elemObj->type());
        SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& elemTypeInfo = (*typeInfoIt).second;

        // Convert element initial value to string.
        String initValStr;
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                const I32 initVal =
                    static_cast<const Element<I8>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::INT16:
            {
                const I16 initVal =
                    static_cast<const Element<I16>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::INT32:
            {
                const I32 initVal =
                    static_cast<const Element<I32>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::INT64:
            {
                const I64 initVal =
                    static_cast<const Element<I64>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::UINT8:
            {
                const I32 initVal =
                    static_cast<const Element<U8>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::UINT16:
            {
                const U16 initVal =
                    static_cast<const Element<U16>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::UINT32:
            {
                const U32 initVal =
                    static_cast<const Element<U32>*>(elemObj)->read();
                initValStr = Autocode::format("%%U", initVal);
                break;
            }

            case ElementType::UINT64:
            {
                const U64 initVal =
                    static_cast<const Element<U64>*>(elemObj)->read();
                initValStr = Autocode::format("%%ULL", initVal);
                break;
            }

            case ElementType::FLOAT32:
            {
                const F32 initVal =
                    static_cast<const Element<F32>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::FLOAT64:
            {
                const F64 initVal =
                    static_cast<const Element<F64>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::BOOL:
            {
                const bool initVal =
                    static_cast<const Element<bool>*>(elemObj)->read();
                initValStr = (initVal ? "true" : "false");
                break;
            }

            default:
                SF_ASSERT(false);
        }

        // Define element backing struct member.
        a("%% %% = %%;", elemTypeInfo.name, elem->name, initValStr);

        // Save element object and pointer definitions for appending to the
        // autocode once the struct definition is done.
        elemDefs.push_back(
            Autocode::format("static Element<%%> elemObj%%(localBacking.%%);",
            elemTypeInfo.name, elem->name, elem->name));
        elemDefs.push_back(
            Autocode::format("static Element<%%>* elem%% = &elemObj%%;",
            elemTypeInfo.name, elem->name, elem->name));

        // Add local element object to the set of referenced elements. This will
        // prevent StateMachineAutocoder::codeElementLookup() from generating
        // code which looks up the element in the global state vector.
        kWs.refElems.insert(elemObj);
    }

    a.decreaseIndent();
    a("} localBacking;");
    a();

    // Append element definitions.
    for (const String& elemDef : elemDefs)
    {
        a(elemDef);
    }

    a();
}

String StateMachineAutocoder::codeConstExprNode(
    const IExpression* const kNode,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kNode != nullptr);
    SF_ASSERT(kNode->nodeType() == IExpression::CONST);

    Autocode& a = kAutocode;

    // Generate a unique identifier for the node.
    const String nodeId = Autocode::format("node%%", kWs.exprNodeCnt++);

    // Define node.
    switch (kNode->type())
    {
        case ElementType::INT8:
        {
            const ConstExprNode<I8>* const node =
                dynamic_cast<const ConstExprNode<I8>*>(kNode);
            a("static ConstExprNode<I8> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::INT16:
        {
            const ConstExprNode<I16>* const node =
                dynamic_cast<const ConstExprNode<I16>*>(kNode);
            a("static ConstExprNode<I16> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::INT32:
        {
            const ConstExprNode<I32>* const node =
                dynamic_cast<const ConstExprNode<I32>*>(kNode);
            a("static ConstExprNode<I32> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::INT64:
        {
            const ConstExprNode<I64>* const node =
                dynamic_cast<const ConstExprNode<I64>*>(kNode);
            a("static ConstExprNode<I64> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::UINT8:
        {
            const ConstExprNode<U8>* const node =
                dynamic_cast<const ConstExprNode<U8>*>(kNode);
            a("static ConstExprNode<U8> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::UINT16:
        {
            const ConstExprNode<U16>* const node =
                dynamic_cast<const ConstExprNode<U16>*>(kNode);
            a("static ConstExprNode<U16> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::UINT32:
        {
            const ConstExprNode<U32>* const node =
                dynamic_cast<const ConstExprNode<U32>*>(kNode);
            a("static ConstExprNode<U32> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::UINT64:
        {
            const ConstExprNode<U64>* const node =
                dynamic_cast<const ConstExprNode<U64>*>(kNode);
            a("static ConstExprNode<U64> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::FLOAT32:
        {
            const ConstExprNode<F32>* const node =
                dynamic_cast<const ConstExprNode<F32>*>(kNode);
            a("static ConstExprNode<F32> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::FLOAT64:
        {
            const ConstExprNode<F64>* const node =
                dynamic_cast<const ConstExprNode<F64>*>(kNode);
            a("static ConstExprNode<F64> %%(%%);", nodeId, node->val());
            break;
        }

        case ElementType::BOOL:
        {
            const ConstExprNode<bool>* const node =
                dynamic_cast<const ConstExprNode<bool>*>(kNode);
            a("static ConstExprNode<bool> %%(%%);", nodeId, node->val());
            break;
        }

        default:
            SF_ASSERT(false);
    }

    // Return address of defined node.
    return Autocode::format("&%%", nodeId);
}

void StateMachineAutocoder::codeElementLookup(
    Autocode& kAutocode,
    const IElement* const kElemObj,
    const TypeInfo& kElemTypeInfo,
    const String kElemName,
    StateMachineAutocoder::Workspace& kWs)
{
    Autocode& a = kAutocode;

    // If this is the first time the element is being referenced, generate code
    // which defines a pointer to it.
    auto elemRefIt = kWs.refElems.find(kElemObj);
    if (elemRefIt == kWs.refElems.end())
    {
        a("Element<%%>* elem%% = nullptr;", kElemTypeInfo.name, kElemName);
        a("res = kSv.getElement(\"%%\", elem%%);", kElemName, kElemName);
        a("if (res != SUCCESS)");
        a("{");
        a.increaseIndent();
        a("return res;");
        a.decreaseIndent();
        a("}");
        kWs.refElems.insert(kElemObj);
    }
}

String StateMachineAutocoder::codeElementExprNode(
    const IExpression* const kNode,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kNode != nullptr);
    SF_ASSERT(kNode->nodeType() == IExpression::ELEMENT);

    Autocode& a = kAutocode;

    // Generate a unique identifier for the node.
    const String nodeId = Autocode::format("node%%", kWs.exprNodeCnt++);

    // Look up type info for ExpressionStats template parameter.
    auto typeInfoIt = TypeInfo::fromEnum.find(kNode->type());
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& elemTypeInfo = (*typeInfoIt).second;

    // Node class is an instantiation of the ElementExprNode template matching
    // the element's type.
    const String classId = Autocode::format("ElementExprNode<%%>",
                                            elemTypeInfo.name);

    // Find element name based on the address of the element object.
    const IElementExprNode* const inode =
        dynamic_cast<const IElementExprNode*>(kNode);
    const IElement* elemObj = &inode->elem();
    const String elemName = elemNameFromAddr(elemObj, kWs);

    // Generate code for element lookup if necessary.
    codeElementLookup(a, elemObj, elemTypeInfo, elemName, kWs);

    // Define node.
    a("static %% %%(*elem%%);", classId, nodeId, elemName);

    // Return address of defined node.
    return Autocode::format("&%%", nodeId);
}

String StateMachineAutocoder::codeBinOpExprNode(
    const IExpression* const kNode,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kNode != nullptr);
    SF_ASSERT(kNode->nodeType() == IExpression::BIN_OP);

    Autocode& a = kAutocode;

    // Generate a unique identifier for the node.
    const String nodeId = Autocode::format("node%%", kWs.exprNodeCnt++);

    // Generate code for operation LHS expression. This requires downcasting to
    // IOpExprNode to get a pointer to the LHS expression root node.
    const IOpExprNode* const iopNode = dynamic_cast<const IOpExprNode*>(kNode);
    const String lhsAddr =
        StateMachineAutocoder::codeExpression(iopNode->lhs(), a, kWs);

    // Generate code for operation RHS expression.
    const String rhsAddr =
        StateMachineAutocoder::codeExpression(iopNode->rhs(), a, kWs);

    // Get address of operation function and use it to look up the identifier of
    // the operation function.
    const void* const opFuncPtr = iopNode->op();
    auto opIdIt = StateMachineAutocoder::opFuncIds.find(opFuncPtr);
    SF_ASSERT(opIdIt != StateMachineAutocoder::opFuncIds.end());
    const String opFuncId = (*opIdIt).second;

    // Look up type info for operation evaluation type.
    auto typeInfoIt = TypeInfo::fromEnum.find(kNode->type());
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& evalTypeInfo = (*typeInfoIt).second;

    // Look up type info for LHS evaluation type. Since the BinOpExprNode
    // template uses the same type for both operands, the LHS evaluation type
    // is the same as the RHS evaluation type.
    typeInfoIt = TypeInfo::fromEnum.find(iopNode->lhs()->type());
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& lhsTypeInfo = (*typeInfoIt).second;

    // Define node.
    a("static BinOpExprNode<%%, %%> %%(%%, *%%, *%%);",
      evalTypeInfo.name, lhsTypeInfo.name, nodeId, opFuncId, lhsAddr, rhsAddr);

    // Return address of defined node.
    return Autocode::format("&%%", nodeId);
}

String StateMachineAutocoder::codeUnaryOpExprNode(
    const IExpression* const kNode,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kNode != nullptr);
    SF_ASSERT(kNode->nodeType() == IExpression::UNARY_OP);

    Autocode& a = kAutocode;

    // Generate a unique identifier for the node.
    const String nodeId = Autocode::format("node%%", kWs.exprNodeCnt++);

    // Generate code for operation RHS expression. This requires downcasting to
    // IOpExprNode to get a pointer to the RHS expression root node.
    const IOpExprNode* const iopNode = dynamic_cast<const IOpExprNode*>(kNode);
    const String rhsAddr =
        StateMachineAutocoder::codeExpression(iopNode->rhs(), a, kWs);

    // Get address of operation function and use it to look up the identifier of
    // the operation function.
    const void* const opFuncPtr = iopNode->op();
    auto opIdIt = StateMachineAutocoder::opFuncIds.find(opFuncPtr);
    SF_ASSERT(opIdIt != StateMachineAutocoder::opFuncIds.end());
    const String opFuncId = (*opIdIt).second;

    // Look up type info for operation evaluation type.
    auto typeInfoIt = TypeInfo::fromEnum.find(kNode->type());
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& evalTypeInfo = (*typeInfoIt).second;

    // Look up type info for RHS evaluation type.
    typeInfoIt = TypeInfo::fromEnum.find(iopNode->rhs()->type());
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& rhsTypeInfo = (*typeInfoIt).second;

    // Define node. Note that we only instantiate the template with the first
    // parameter; the compiler will be able to deduce the remaining parameters
    // based on the signature of the operation function passed to the
    // constructor.
    a("static UnaryOpExprNode<%%, %%> %%(%%, *%%);",
      evalTypeInfo.name, rhsTypeInfo.name, nodeId, opFuncId, rhsAddr);

    // Return address of defined node.
    return Autocode::format("&%%", nodeId);
}

String StateMachineAutocoder::codeExprStatsNode(
    const IExpression* const kNode,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kNode != nullptr);

    Autocode& a = kAutocode;

    // Generate a unique identifier for the node.
    const String nodeId = Autocode::format("node%%", kWs.exprNodeCnt++);

    // Generate code for expression which stats are computed on. This requires
    // downcasting to IExprStatsNode so that we can get a pointer to the
    // IExpression which stats are being computed on.
    const IExprStatsNode* const nodeNarrow =
        dynamic_cast<const IExprStatsNode*>(kNode);
    const IExpressionStats& stats = nodeNarrow->stats();
    const IExpression& statsExpr = stats.expr();
    const String statsExprAddr =
        StateMachineAutocoder::codeExpression(&statsExpr, a, kWs);

    // Determine node class identifier.
    auto nodeIdIt =
        StateMachineAutocoder::exprStatNodeIds.find(kNode->nodeType());
    SF_ASSERT(nodeIdIt != StateMachineAutocoder::exprStatNodeIds.end());
    const String classId = (*nodeIdIt).second;
    SF_ASSERT(classId.size() > 0);

    // Look up type info for ExpressionStats template parameter.
    const ElementType statsType = stats.type();
    auto typeInfoIt = TypeInfo::fromEnum.find(statsType);
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& statsTypeInfo = (*typeInfoIt).second;

    // Define arrays for node ExpressionStats to use.
    a("static %% %%ArrA[%%];", statsTypeInfo.name, nodeId, stats.size());
    a("static %% %%ArrB[%%];", statsTypeInfo.name, nodeId, stats.size());

    // Define node ExpressionStats.
    const String statsId = Autocode::format("stats%%", kWs.statsCnt++);
    a("static ExpressionStats<%%> %%(*%%, %%ArrA, %%ArrB, %%);",
      statsTypeInfo.name, statsId, statsExprAddr, nodeId, nodeId, stats.size());

    // Define node.
    a("static %% %%(%%);", classId, nodeId, statsId);

    // Return address of defined node.
    return Autocode::format("&%%", nodeId);
}

String StateMachineAutocoder::codeExpression(
    const IExpression* const kExpr,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    // Node is null, so we fell off the expression tree.
    if (kExpr == nullptr)
    {
        return "nullptr";
    }

    Autocode& a = kAutocode;

    switch (kExpr->nodeType())
    {
        // ConstExprNode
        case IExpression::CONST:
            return StateMachineAutocoder::codeConstExprNode(kExpr, a, kWs);

        // ElementExprNode
        case IExpression::ELEMENT:
            return StateMachineAutocoder::codeElementExprNode(kExpr, a, kWs);

        // BinOpExprNode
        case IExpression::BIN_OP:
            return StateMachineAutocoder::codeBinOpExprNode(kExpr, a, kWs);

        // UnaryOpExprNode
        case IExpression::UNARY_OP:
            return StateMachineAutocoder::codeUnaryOpExprNode(kExpr, a, kWs);

        // IExprStatsNode
        case IExpression::ROLL_AVG:
        case IExpression::ROLL_MEDIAN:
        case IExpression::ROLL_MIN:
        case IExpression::ROLL_MAX:
        case IExpression::ROLL_RANGE:
            return StateMachineAutocoder::codeExprStatsNode(kExpr, a, kWs);

        default:
            // Unknown expression node type.
            SF_ASSERT(false);
    }

    return "(unknown expression node)";
}

String StateMachineAutocoder::codeAction(const IAction* const kAction,
                                         Autocode& kAutocode,
                                         StateMachineAutocoder::Workspace& kWs)
{
    // Action is null, so represent as a null pointer in the owning structure.
    if (kAction == nullptr)
    {
        return "nullptr";
    }

    Autocode& a = kAutocode;

    // Generate a unique identifier for the action.
    const String actId = Autocode::format("act%%", kWs.actCnt++);

    // IAction needs no special methods for RTTI; the child type can be
    // determined solely from the value of IAction::destState, which is
    // StateMachine::NO_STATE only for assignment actions.
    if (kAction->destState == StateMachine::NO_STATE)
    {
        // Downcast to IAssignmentAction so that we can get pointers to the
        // LHS element and RHS expression root node.
        const IAssignmentAction* const iact =
            static_cast<const IAssignmentAction*>(kAction);

        // Generate code for RHS expression.
        const String lhsAddr =
            StateMachineAutocoder::codeExpression(&iact->expr(), a, kWs);

        // Look up element name using the address of the element object.
        const IElement* const elemObj = &iact->elem();
        const String elemName =
            StateMachineAutocoder::elemNameFromAddr(elemObj, kWs);

        // Look up type info for element, which matches the AssignmentAction
        // template parameter.
        auto typeInfoIt = TypeInfo::fromEnum.find(iact->elem().type());
        SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& elemTypeInfo = (*typeInfoIt).second;

        // Generate code for element lookup if necessary.
        StateMachineAutocoder::codeElementLookup(a,
                                                 elemObj,
                                                 elemTypeInfo,
                                                 elemName,
                                                 kWs);

        // Define assignment action.
        a("static AssignmentAction<%%> %%(*elem%%, *%%);",
          elemTypeInfo.name, actId, elemName, lhsAddr);
    }
    else
    {
        // Define transition action.
        a("static TransitionAction %%(%%);", actId, kAction->destState);
    }

    // Return address of defined action.
    return Autocode::format("&%%", actId);
}

String StateMachineAutocoder::codeBlock(const StateMachine::Block* const kBlock,
                                        Autocode& kAutocode,
                                        StateMachineAutocoder::Workspace& kWs)
{
    // Block is null, so represent as a null pointer in the owning structure.
    if (kBlock == nullptr)
    {
        return "nullptr";
    }

    Autocode& a = kAutocode;

    // Generate a unique ID for the block.
    const String blockId = Autocode::format("block%%", kWs.blockCnt++);

    // Generate code for the block guard expression.
    const String guardAddr =
        StateMachineAutocoder::codeExpression(kBlock->guard, a, kWs);

    // Generate code for if branch block.
    const String ifAddr =
        StateMachineAutocoder::codeBlock(kBlock->ifBlock, a, kWs);

    // Generate code for else branch block.
    const String elseAddr =
        StateMachineAutocoder::codeBlock(kBlock->elseBlock, a, kWs);

    // Generate code for block action.
    const String actionAddr =
        StateMachineAutocoder::codeAction(kBlock->action, a, kWs);

    // Generate code for next block.
    const String nextAddr =
        StateMachineAutocoder::codeBlock(kBlock->next, a, kWs);

    // Define block.
    a("static StateMachine::Block %%{%%, %%, %%, %%, %%};",
      blockId, guardAddr, ifAddr, elseAddr, actionAddr, nextAddr);

    // Return address of defined block.
    return Autocode::format("&%%", blockId);
}

void StateMachineAutocoder::codeState(
    const StateMachine::StateConfig* const kState,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kState != nullptr);

    Autocode& a = kAutocode;

    a("// State %% config", kState->id);

    // Generate code for entry block.
    const String entryAddr =
        StateMachineAutocoder::codeBlock(kState->entry, a, kWs);

    // Generate code for step block.
    const String stepAddr =
        StateMachineAutocoder::codeBlock(kState->step, a, kWs);

    // Generate code for exit block.
    const String exitAddr =
        StateMachineAutocoder::codeBlock(kState->exit, a, kWs);

    // Define state config.
    a("static StateMachine::StateConfig state%%Config = {%%, %%, %%, %%};",
      kState->id, kState->id, entryAddr, stepAddr, exitAddr);
    a();
}
