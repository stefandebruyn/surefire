#include "sf/config/Autocode.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/core/Assert.hpp"

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

    // Get state vector elements from state vector.
    StateMachineAutocoder::codeStateVectorElems(parse, a, ws);

    // Define local state vector elements.
    StateMachineAutocoder::codeLocalStateVector(parse, kSmAsm, a, ws);

    // Define state configs.
    for (const StateMachineParse::StateParse& state : parse->states)
    {
        StateMachineAutocoder::codeState(state, kSmAsm, a, ws);
        a();
    }

    // Define state config array.
    a("static StateMachine::StateConfig stateConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const String& state : ws.stateNames)
    {
        a("state%%Config,", state);
    }

    a("{StateMachine::NO_STATE, nullptr, nullptr, nullptr}"); // Null terminator
    a.decreaseIndent();
    a("};");
    a();

    // Define expression stats array.
    String exprStatsArrAddr = "nullptr";
    if (ws.exprStatIds.size() > 0)
    {
        a("static IExpressionStats* exprStats[] =");
        a("{");
        a.increaseIndent();

        for (const String& id : ws.exprStatIds)
        {
            a("&%%,", id);
        }

        a("nullptr"); // Null terminator
        a.decreaseIndent();
        a("};");
        a();

        exprStatsArrAddr = "exprStats";
    }

    // Return state machine config to caller.
    a("static StateMachine::Config smConfig = {%%, %%, %%, %%, %%};",
      "&elemstate", "&elemT", "&elemtime", "stateConfigs", exprStatsArrAddr);
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

    const Ref<StateVector> localSv = kSmAsm->localStateVector();
    SF_ASSERT(localSv != nullptr);
    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        // Get element initial value as computed in the state machine assembly.
        IElement* elemObj = nullptr;
        const Result res = localSv->getIElement(elem.tokName.str.c_str(),
                                                elemObj);
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
    const ElementType kEvalType,
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
            kParse->right, kSmAsm, ElementType::FLOAT64, a, kWs);

        if (!kParse->data.opInfo->unary)
        {
            // Node is a binary operator - code LHS expression as well.
            const String lhsAddr = StateMachineAutocoder::codeExpression(
                kParse->left, kSmAsm, ElementType::FLOAT64, a, kWs);

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

    // If specified evaluation type is not F64, add a cast.
    if (kEvalType != ElementType::FLOAT64)
    {
        // Look up cast type info.
        auto typeInfoIt = TypeInfo::fromEnum.find(kEvalType);
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
