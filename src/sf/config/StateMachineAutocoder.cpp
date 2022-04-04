#include "sf/config/Autocode.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/core/Assert.hpp"
#include "sf/core/Expression.hpp"

static Map<const void*, String> gOpFuncIds =
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

static Map<IExpression::NodeType, String> gExprStatNodeIds =
{
    {IExpression::ROLL_AVG, "RollAvgNode"},
    {IExpression::ROLL_MEDIAN, "RollMedianNode"},
    {IExpression::ROLL_MIN, "RollMinNode"},
    {IExpression::ROLL_MAX, "RollMaxNode"},
    {IExpression::ROLL_RANGE, "RollRangeNode"}
};

/////////////////////////////////// Public /////////////////////////////////////

static String elemNameFromAddr(const IElement* const kAddr,
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

static void codeLocalStateVector(Autocode& kAutocode,
                                 StateMachineAutocoder::Workspace& kWs)
{
    Autocode& a = kAutocode;

    const Ref<const StateVectorAssembly> localSvAsm = kWs.smAsm->mWs.localSvAsm;
    SF_ASSERT(localSvAsm != nullptr);
    const StateVector::Config localSvConfig = localSvAsm->config();

    a("// Local state vector");
    a("static struct");
    a("{");
    a.increaseIndent();

    Vec<String> elemDefs;
    for (const StateVector::ElementConfig* elem = localSvConfig.elems;
         elem->name != nullptr;
         ++elem)
    {
        const IElement* const elemObj = elem->elem;
        SF_ASSERT(elemObj != nullptr);
        auto typeInfoIt = TypeInfo::fromEnum.find(elemObj->type());
        SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& elemTypeInfo = (*typeInfoIt).second;

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

        a("%% %% = %%;", elemTypeInfo.name, elem->name, initValStr);

        elemDefs.push_back(
            Autocode::format("static Element<%%> elemObj%%(localBacking.%%);",
            elemTypeInfo.name, elem->name, elem->name));
        elemDefs.push_back(
            Autocode::format("static Element<%%>* elem%% = &elemObj%%;",
            elemTypeInfo.name, elem->name, elem->name));
        kWs.refElems.insert(elemObj);
    }

    a.decreaseIndent();
    a("} localBacking;");
    a();

    for (const String& elemDef : elemDefs)
    {
        a(elemDef);
    }

    a();
}

static String codeConstExprNode(const IExpression* const kNode,
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

static void codeElementLookup(Autocode& kAutocode,
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

static String codeElementExprNode(const IExpression* const kNode,
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

static String codeExpression(const IExpression* const kExpr,
                             Autocode& kAutocode,
                             StateMachineAutocoder::Workspace& kWs);

static String codeBinOpExprNode(const IExpression* const kNode,
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
    const String lhsAddr = codeExpression(iopNode->lhs(), a, kWs);

    // Generate code for operation RHS expression.
    const String rhsAddr = codeExpression(iopNode->rhs(), a, kWs);

    // Get address of operation function and use it to look up the identifier of
    // the operation function.
    const void* const opFuncPtr = iopNode->op();
    auto opIdIt = gOpFuncIds.find(opFuncPtr);
    SF_ASSERT(opIdIt != gOpFuncIds.end());
    const String opFuncId = (*opIdIt).second;

    // Look up type info for operation evaluation type.
    auto typeInfoIt = TypeInfo::fromEnum.find(kNode->type());
    SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
    const TypeInfo& evalTypeInfo = (*typeInfoIt).second;

    // Define node. Note that we only instantiate the template with the first
    // parameter; the compiler will be able to deduce the remaining parameters
    // based on the signature of the operation function passed to the
    // constructor.
    a("static BinOpExprNode<%%> %%(%%, *%%, *%%);",
      evalTypeInfo.name, nodeId, opFuncId, lhsAddr, rhsAddr);

    // Return address of defined node.
    return Autocode::format("&%%", nodeId);
}

static String codeUnaryOpExprNode(const IExpression* const kNode,
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
    const String rhsAddr = codeExpression(iopNode->rhs(), a, kWs);

    // Get address of operation function and use it to look up the identifier of
    // the operation function.
    const void* const opFuncPtr = iopNode->op();
    auto opIdIt = gOpFuncIds.find(opFuncPtr);
    SF_ASSERT(opIdIt != gOpFuncIds.end());
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

static String codeExprStatsNode(const IExpression* const kNode,
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
    const String statsExprAddr = codeExpression(&statsExpr, a, kWs);

    // Determine node class identifier.
    const String classId = gExprStatNodeIds[kNode->nodeType()];
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

static String codeExpression(const IExpression* const kExpr,
                             Autocode& kAutocode,
                             StateMachineAutocoder::Workspace& kWs)
{
    // Node is null, so we fell off the expression tree.
    if (kExpr == nullptr)
    {
        return "nullptr";
    }

    switch (kExpr->nodeType())
    {
        // ConstExprNode
        case IExpression::CONST:
            return codeConstExprNode(kExpr, kAutocode, kWs);

        // ElementExprNode
        case IExpression::ELEMENT:
            return codeElementExprNode(kExpr, kAutocode, kWs);

        // BinOpExprNode
        case IExpression::BIN_OP:
            return codeBinOpExprNode(kExpr, kAutocode, kWs);

        // UnaryOpExprNode
        case IExpression::UNARY_OP:
            return codeUnaryOpExprNode(kExpr, kAutocode, kWs);

        // IExprStatsNode
        case IExpression::ROLL_AVG:
        case IExpression::ROLL_MEDIAN:
        case IExpression::ROLL_MIN:
        case IExpression::ROLL_MAX:
        case IExpression::ROLL_RANGE:
            return codeExprStatsNode(kExpr, kAutocode, kWs);

        default:
            // Unknown expression node type.
            SF_ASSERT(false);
    }

    return "(unknown expression node)";
}

static String codeAction(const IAction* const kAction,
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
        const String lhsAddr = codeExpression(&iact->expr(), a, kWs);

        // Look up element name using the address of the element object.
        const IElement* const elemObj = &iact->elem();
        const String elemName = elemNameFromAddr(elemObj, kWs);

        // Look up type info for element, which matches the AssignmentAction
        // template parameter.
        auto typeInfoIt = TypeInfo::fromEnum.find(iact->elem().type());
        SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& elemTypeInfo = (*typeInfoIt).second;

        // Generate code for element lookup if necessary.
        codeElementLookup(a, elemObj, elemTypeInfo, elemName, kWs);

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

static String codeBlock(const StateMachine::Block* const kBlock,
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
    const String guardAddr = codeExpression(kBlock->guard, a, kWs);

    // Generate code for if branch block.
    const String ifAddr = codeBlock(kBlock->ifBlock, a, kWs);

    // Generate code for else branch block.
    const String elseAddr = codeBlock(kBlock->elseBlock, a, kWs);

    // Generate code for block action.
    const String actionAddr = codeAction(kBlock->action, a, kWs);

    // Generate code for next block.
    const String nextAddr = codeBlock(kBlock->next, a, kWs);

    // Define block.
    a("static StateMachine::Block %%{%%, %%, %%, %%, %%};",
      blockId, guardAddr, ifAddr, elseAddr, actionAddr, nextAddr);

    // Return address of defined block.
    return Autocode::format("&%%", blockId);
}

static void codeState(const StateMachine::StateConfig* const kState,
                      Autocode& kAutocode,
                      StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kState != nullptr);

    Autocode& a = kAutocode;

    a("// State %% config", kState->id);

    // Generate code for entry block.
    const String entryAddr = codeBlock(kState->entry, a, kWs);

    // Generate code for step block.
    const String stepAddr = codeBlock(kState->step, a, kWs);

    // Generate code for exit block.
    const String exitAddr = codeBlock(kState->exit, a, kWs);

    // Define state config.
    a("static StateMachine::StateConfig state%%Config = {%%, %%, %%, %%};",
      kState->id, kState->id, entryAddr, stepAddr, exitAddr);
    a();
}

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

    // Define local state vector.
    ::codeLocalStateVector(a, ws);

    // Generate code for state configs.
    const StateMachine::Config smConfig = kSmAsm->config();
    for (const StateMachine::StateConfig* state = smConfig.states;
         state->id != StateMachine::NO_STATE;
         ++state)
    {
        ::codeState(state, a, ws);
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

    // a("Result res = SUCCESS;");
    // a();

    // // Get state vector elements from state vector.
    // StateMachineAutocoder::codeStateVectorElems(parse, a, ws);

    // // Define local state vector elements.
    // StateMachineAutocoder::codeLocalStateVector(parse, kSmAsm, a, ws);

    // // Define state configs.
    // for (const StateMachineParse::StateParse& state : parse->states)
    // {
    //     StateMachineAutocoder::codeState(state, kSmAsm, a, ws);
    //     a();
    // }

    // // Define state config array.
    // a("static StateMachine::StateConfig stateConfigs[] =");
    // a("{");
    // a.increaseIndent();

    // for (const String& state : ws.stateNames)
    // {
    //     a("state%%Config,", state);
    // }

    // a("{StateMachine::NO_STATE, nullptr, nullptr, nullptr}"); // Null terminator
    // a.decreaseIndent();
    // a("};");
    // a();

    // // Define expression stats array.
    // String exprStatsArrAddr = "nullptr";
    // if (ws.exprStatIds.size() > 0)
    // {
    //     a("static IExpressionStats* exprStats[] =");
    //     a("{");
    //     a.increaseIndent();

    //     for (const String& id : ws.exprStatIds)
    //     {
    //         a("&%%,", id);
    //     }

    //     a("nullptr"); // Null terminator
    //     a.decreaseIndent();
    //     a("};");
    //     a();

    //     exprStatsArrAddr = "exprStats";
    // }

    // // Find the parses of the global time and state elements.
    // const StateMachineParse::StateVectorElementParse* elemGlobalTimeParse
    //     = nullptr;
    // const StateMachineParse::StateVectorElementParse* elemStateParse = nullptr;
    // for (const StateMachineParse::StateVectorElementParse& elem :
    //      parse->svElems)
    // {
    //     if (elem.tokAlias.str == LangConst::elemGlobalTime)
    //     {
    //         SF_ASSERT(elemGlobalTimeParse == nullptr);
    //         elemGlobalTimeParse = &elem;
    //     }

    //     if (elem.tokAlias.str == LangConst::elemState)
    //     {
    //         SF_ASSERT(elemStateParse == nullptr);
    //         elemStateParse = &elem;
    //     }
    // }

    // SF_ASSERT(elemGlobalTimeParse != nullptr)
    // SF_ASSERT(elemStateParse != nullptr);

    // // Define state machine config and return to caller.
    // a("static StateMachine::Config smConfig = {&elem%%, &elemT, &elem%%, stateConfigs, %%};",
    //   elemStateParse->tokName.str,
    //   elemGlobalTimeParse->tokName.str,
    //   exprStatsArrAddr);
    // a("kSmConfig = smConfig;");
    // a();

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

Map<String, String> StateMachineAutocoder::mFuncNodeTypeNames =
{
    {LangConst::funcRollAvg, "RollAvgNode"},
    {LangConst::funcRollMedian, "RollMedianNode"},
    {LangConst::funcRollMin, "RollMinNode"},
    {LangConst::funcRollMax, "RollMaxNode"},
    {LangConst::funcRollRange, "RollRangeNode"}
};

void StateMachineAutocoder::codeLocalStateVector(
    const Ref<const StateMachineParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kParse != nullptr);

    Autocode& a = kAutocode;

    // Define backing storage struct.
    a("// Local elements");
    a("static struct");
    a("{");
    a.increaseIndent();

    StateVector& localSv = kSmAsm->localStateVector();
    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        // Get element initial value as computed in the state machine assembly.
        IElement* elemObj = nullptr;
        const Result res = localSv.getIElement(elem.tokName.str.c_str(),
                                               elemObj);
        (void) res;
        SF_ASSERT(res == SUCCESS);
        SF_ASSERT(elemObj != nullptr);
        String initValStr;
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                const I32 initVal = static_cast<Element<I8>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::INT16:
            {
                const I16 initVal = static_cast<Element<I16>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::INT32:
            {
                const I32 initVal = static_cast<Element<I32>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::INT64:
            {
                const I64 initVal = static_cast<Element<I64>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::UINT8:
            {
                const I32 initVal = static_cast<Element<U8>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::UINT16:
            {
                const U16 initVal = static_cast<Element<U16>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::UINT32:
            {
                const U32 initVal = static_cast<Element<U32>*>(elemObj)->read();
                initValStr = Autocode::format("%%U", initVal);
                break;
            }

            case ElementType::UINT64:
            {
                const U64 initVal = static_cast<Element<U64>*>(elemObj)->read();
                initValStr = Autocode::format("%%ULL", initVal);
                break;
            }

            case ElementType::FLOAT32:
            {
                const F32 initVal = static_cast<Element<F32>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::FLOAT64:
            {
                const F64 initVal = static_cast<Element<F64>*>(elemObj)->read();
                initValStr = Autocode::format("%%", initVal);
                break;
            }

            case ElementType::BOOL:
            {
                const bool initVal =
                    static_cast<Element<bool>*>(elemObj)->read();
                initValStr = (initVal ? "true" : "false");
                break;
            }

            default:
                SF_ASSERT(false);
        }

        // Define element struct member with initial value.
        SF_ASSERT(initValStr.size() > 0);
        SF_ASSERT(elem.tokType.typeInfo != nullptr);
        a("%% %% = %%;",
          elem.tokType.typeInfo->name, elem.tokName.str, initValStr);

        // Map element name to type info for later lookup.
        kWs.elemTypeInfo[elem.tokName.str] = elem.tokType.typeInfo;

        // Map element name to itself in the original elements name map since
        // local elements can't be aliased.
        kWs.elemTrueNames[elem.tokName.str] = elem.tokName.str;
    }

    // Add backing for built-in state time element.
    a("U64 T = 0;");
    kWs.elemTrueNames["T"] = "T";

    a.decreaseIndent();
    a("} localBacking;");
    a();

    // Define user element objects.
    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        SF_ASSERT(elem.tokType.typeInfo != nullptr);
        a("static Element<%%> elem%%(localBacking.%%);",
          elem.tokType.typeInfo->name,
          elem.tokName.str,
          elem.tokName.str);
    }

    // Define built-in state time element object.
    kWs.elemTypeInfo["T"] = &TypeInfo::u64;
    a("static Element<U64> elemT(localBacking.T);");
    a();
}

void StateMachineAutocoder::codeStateVectorElems(
    const Ref<const StateMachineParse> kParse,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kParse != nullptr);
    Autocode& a = kAutocode;
 
    // Add comment above state vector elements for clarity.
    a("// State vector elements");

    for (const StateMachineParse::StateVectorElementParse& elem :
         kParse->svElems)
    {
        // Declare a pointer for the element and populate it via a call to
        // StateVector::getElement, returning early on error.
        SF_ASSERT(elem.tokType.typeInfo != nullptr);
        a("static Element<%%>* pElem%% = nullptr;",
          elem.tokType.typeInfo->name, elem.tokName.str);
        a("res = kSv.getElement(\"%%\", pElem%%);",
          elem.tokName.str, elem.tokName.str);
        a("if (res != SUCCESS)");
        a("{");
        a.increaseIndent();
        a("return res;");
        a.decreaseIndent();
        a("}");
        a("static Element<%%>& elem%% = *pElem%%;",
          elem.tokType.typeInfo->name, elem.tokName.str, elem.tokName.str);
        a();

        // Map element name and alias to type info for later lookup. Also
        // add mappings for retrieving the original element name.
        kWs.elemTypeInfo[elem.tokName.str] = elem.tokType.typeInfo;
        kWs.elemTrueNames[elem.tokName.str] = elem.tokName.str;
        if (elem.tokAlias.str.size() > 0)
        {
            kWs.elemTypeInfo[elem.tokAlias.str] = elem.tokType.typeInfo;
            kWs.elemTrueNames[elem.tokAlias.str] = elem.tokName.str;
        }
    }
}

String StateMachineAutocoder::codeFunctionCall(
    const Ref<const ExpressionParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kSmAsm != nullptr);
    SF_ASSERT(kParse != nullptr);
    SF_ASSERT(kParse->func);
    Autocode& a = kAutocode;

    // Generate code for argument expressions.
    Vec<String> argAddrs;
    Ref<const ExpressionParse> node = kParse;
    while (node->left != nullptr)
    {
        argAddrs.push_back(
            StateMachineAutocoder::codeExpression(node->left->right,
                                                  kSmAsm,
                                                  ElementType::FLOAT64,
                                                  kAutocode,
                                                  kWs));
        node = node->left;
    }

    // Get size of rolling window as computed in state machine assembly.
    const Ref<Vec<IExpressionStats*>> asmExprStats = kSmAsm->mWs.exprStatArr;
    SF_ASSERT(asmExprStats != nullptr);
    SF_ASSERT((*asmExprStats)[kWs.statsCnt] != nullptr);
    const U32 statsSize = (*asmExprStats)[kWs.statsCnt]->size();

    // Define expression stats object for function node.
    const String statsId = Autocode::format("stats%%", kWs.statsCnt++);
    a("static F64 %%ArrA[%%];", statsId, statsSize);
    a("static F64 %%ArrB[%%];", statsId, statsSize);
    a("static ExpressionStats<F64> %%(*%%, %%ArrA, %%ArrB, %%);",
      statsId, argAddrs[0], statsId, statsId, statsSize);

    // Define function node.
    const String nodeTypeName = mFuncNodeTypeNames[kParse->data.str];
    const String nodeId = Autocode::format("node%%", kWs.exprNodeCnt);
    SF_ASSERT(nodeTypeName.size() > 0);
    a("static %% %%(%%);", nodeTypeName, nodeId, statsId);

    // Save expression stats address in workspace for later use in the stats
    // array used in the state machine config.
    kWs.exprStatIds.push_back(statsId);

    // Return address of function node.
    return ("&" + nodeId);
}

String StateMachineAutocoder::codeExpression(
    const Ref<const ExpressionParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    const ElementType kCastType,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kSmAsm != nullptr);
    Autocode& a = kAutocode;

    // Base case: node is null, so we fell off the tree.
    if (kParse == nullptr)
    {
        return "nullptr";
    }

    if (kParse->func)
    {
        // Generate function call code.
        StateMachineAutocoder::codeFunctionCall(kParse, kSmAsm, a, kWs);
    }
    else if (kParse->data.type == Token::CONSTANT)
    {
        // Expression node is a constant - convert constant value to string.
        String valStr;
        if (kParse->data.str == LangConst::constantTrue)
        {
            valStr = "true";
        }
        else if (kParse->data.str == LangConst::constantFalse)
        {
            valStr = "false";
        }
        else
        {
            valStr = kParse->data.str;
            // Ensure numeric constants are autocoded as double literals so
            // the compiler doesn't warn about signedness, overflow, etc.
            if (valStr.find('.') == std::string::npos)
            {
                valStr += ".0";
            }
        }

        // Define constant node.
        a("static ConstExprNode<F64> node%%(%%);", kWs.exprNodeCnt, valStr);
    }
    else if (kParse->data.type == Token::IDENTIFIER)
    {
        // Expression node is an elment reference - define node which evaluates
        // to element.
        const TypeInfo* const elemTypeInfo = kWs.elemTypeInfo[kParse->data.str];
        const String elemName = kWs.elemTrueNames[kParse->data.str];
        SF_ASSERT(elemTypeInfo != nullptr);
        SF_ASSERT(elemName.size() > 0);
        a("static ElementExprNode<%%> node%%(elem%%);",
          elemTypeInfo->name, kWs.exprNodeCnt, elemName);

        // Define node which casts element to F64.
        ++kWs.exprNodeCnt;
        a("static UnaryOpExprNode<F64, %%> node%%(safeCast<F64, %%>, node%%);",
          elemTypeInfo->name,
          kWs.exprNodeCnt,
          elemTypeInfo->name,
          (kWs.exprNodeCnt - 1));
    }
    else
    {
        // Expression node is an operator node - code RHS expression, which both
        // unary and binary operators have.
        SF_ASSERT(kParse->data.opInfo != nullptr);
        const String rhsAddr = StateMachineAutocoder::codeExpression(
            kParse->right, kSmAsm, ElementType::NONE, a, kWs);

        if (!kParse->data.opInfo->unary)
        {
            // Node is a binary operator - code LHS expression as well.
            const String lhsAddr = StateMachineAutocoder::codeExpression(
                kParse->left, kSmAsm, ElementType::NONE, a, kWs);

            // Define operator node. We implement the node's operation as a
            // lambda, which the C++ compiler will convert to a function pointer
            // when passed to the node constructor.
            a("static auto node%%Op = [] (F64 l, F64 r) -> F64 { return (l %% r); };",
              kWs.exprNodeCnt, kParse->data.opInfo->cpp);
            a("static BinOpExprNode<F64> node%%(node%%Op, *%%, *%%);",
              kWs.exprNodeCnt, kWs.exprNodeCnt, lhsAddr, rhsAddr);
        }
        else
        {
            // Define unary operator. We again use a lambda for the node's
            // operation.
            a("static auto node%%Op = [] (F64 r) -> F64 { return %%r; };",
              kWs.exprNodeCnt, kParse->data.opInfo->cpp);
            a("static UnaryOpExprNode<F64> node%%(node%%Op, *%%);",
              kWs.exprNodeCnt, kWs.exprNodeCnt, rhsAddr);
        }
    }

    if (kCastType != ElementType::NONE)
    {
        // Look up cast type info.
        auto typeInfoIt = TypeInfo::fromEnum.find(kCastType);
        SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& typeInfo = (*typeInfoIt).second;

        // Define cast node.
        ++kWs.exprNodeCnt;
        a("static UnaryOpExprNode<%%, F64> node%%(safeCast<%%, F64>, node%%);",
          typeInfo.name, kWs.exprNodeCnt, typeInfo.name, (kWs.exprNodeCnt - 1));
    }

    // Return address of expression root node.
    return Autocode::format("&node%%", kWs.exprNodeCnt++);
}

String StateMachineAutocoder::codeAction(
    const Ref<const StateMachineParse::ActionParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kSmAsm != nullptr);
    Autocode& a = kAutocode;

    // If action is null, represent as a null pointer in the owning structure.
    if (kParse == nullptr)
    {
        return "nullptr";
    }

    // Generate unique identifier for action.
    const String actId = Autocode::format("act%%", kWs.actCnt++);

    if (kParse->rhs != nullptr)
    {
        // Action is an assignment. Code RHS expression evaluating to element
        // type.
        const TypeInfo* const rhsTypeInfo =
            kWs.elemTypeInfo[kParse->tokLhs.str];
        SF_ASSERT(rhsTypeInfo != nullptr);
        const String rhsAddr =
            StateMachineAutocoder::codeExpression(
                kParse->rhs, kSmAsm, rhsTypeInfo->enumVal, a, kWs);

        // Define assignment action.
        const String elemName = kWs.elemTrueNames[kParse->tokLhs.str];
        a("static AssignmentAction<%%> %%(elem%%, *%%);",
          rhsTypeInfo->name, actId, elemName, rhsAddr);
    }
    else
    {
        // Action is a transition.
        a("static TransitionAction %%(%%);",
          actId, kWs.stateIds[kParse->tokDestState.str]);
    }

    // Return address of defined action.
    return Autocode::format("&%%", actId);
}

String StateMachineAutocoder::codeBlock(
    const Ref<const StateMachineParse::BlockParse> kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kSmAsm != nullptr);
    Autocode& a = kAutocode;

    // Base case - block is null.
    if (kParse == nullptr)
    {
        return "nullptr";
    }

    // Generate guard code.
    const String guardAddr =
        StateMachineAutocoder::codeExpression(
            kParse->guard, kSmAsm, ElementType::BOOL, a, kWs);

    // Define if branch code.
    const String ifBlockAddr =
        StateMachineAutocoder::codeBlock(kParse->ifBlock, kSmAsm, a, kWs);

    // Generate else branch code.
    const String elseBlockAddr =
        StateMachineAutocoder::codeBlock(kParse->elseBlock, kSmAsm, a, kWs);

    // Generate action code.
    const String actionAddr =
        StateMachineAutocoder::codeAction(kParse->action, kSmAsm, a, kWs);

    // Generate next block code.
    const String nextBlockAddr =
        StateMachineAutocoder::codeBlock(kParse->next, kSmAsm, a, kWs);

    // Define block.
    a("static StateMachine::Block block%%{%%, %%, %%, %%, %%};",
      kWs.blockCnt,
      guardAddr,
      ifBlockAddr,
      elseBlockAddr,
      actionAddr,
      nextBlockAddr);

    // Return block address.
    return Autocode::format("&block%%", kWs.blockCnt++);
}

void StateMachineAutocoder::codeState(
    const StateMachineParse::StateParse& kParse,
    const Ref<const StateMachineAssembly> kSmAsm,
    Autocode& kAutocode,
    StateMachineAutocoder::Workspace& kWs)
{
    SF_ASSERT(kSmAsm != nullptr);
    Autocode& a = kAutocode;

    // Add comment at start of state config definition.
    const String& sectionName = kParse.tokName.str;
    SF_ASSERT(sectionName.size() >= 3);
    const String stateName = sectionName.substr(1, (sectionName.size() - 2));
    kWs.stateNames.push_back(stateName);
    a("// State `%%`", stateName);

    // Generate entry block code.
    const String entryBlockAddr =
        StateMachineAutocoder::codeBlock(kParse.entry, kSmAsm, a, kWs);

    // Generate step block code.
    const String stepBlockAddr =
        StateMachineAutocoder::codeBlock(kParse.step, kSmAsm, a, kWs);

    // Generate exit block code.
    const String exitBlockAddr =
        StateMachineAutocoder::codeBlock(kParse.exit, kSmAsm, a, kWs);

    // Define state config.
    a("static const StateMachine::StateConfig state%%Config{%%, %%, %%, %%};",
      stateName,
      (kWs.stateCnt + 1), // +1 so that state IDs start at 1
      entryBlockAddr,
      stepBlockAddr,
      exitBlockAddr);

    ++kWs.stateCnt;
}
