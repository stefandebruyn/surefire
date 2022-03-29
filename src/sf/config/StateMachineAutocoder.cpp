#include "sf/config/Autocode.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

struct Workspace final
{
    U32 blockCnt;
    U32 exprNodeCnt;
    U32 stateCnt;
    Vec<String> exprStatIds;
    Vec<String> stateNames;
};

static Result codeLocalStateVector(const Ref<const StateMachineParse> kParse,
                                   Autocode& kAutocode)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    Autocode& a = kAutocode;

    // Define backing storage struct.
    a("// Local state vector");
    a("static struct");
    a("{");
    a.increaseIndent();

    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        SF_SAFE_ASSERT(elem.tokType.typeInfo != nullptr);
        a("%% %%;", elem.tokType.typeInfo->name, elem.tokName.str);
    }

    // Add backing for built-in state time element.
    a("U64 T;");

    a.decreaseIndent();
    a("} localBacking;");
    a();

    // Define user element objects.
    for (const StateMachineParse::LocalElementParse& elem : kParse->localElems)
    {
        SF_SAFE_ASSERT(elem.tokType.typeInfo != nullptr);
        a("static Element<%%> elem%%(localBacking.%%);",
          elem.tokType.typeInfo->name,
          elem.tokName.str,
          elem.tokName.str);
    }

    // Define built-in state time element object.
    a("static Element<U64> elemT(localBacking.T);");
    a();

    return SUCCESS;
}

static Result codeStateVectorElems(const Ref<const StateMachineParse> kParse,
                                   Autocode& kAutocode)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    Autocode& a = kAutocode;

    a("// Referenced state vector elements");

    for (const StateMachineParse::StateVectorElementParse& elem :
         kParse->svElems)
    {
        SF_SAFE_ASSERT(elem.tokType.typeInfo != nullptr);
        a("static Element<%%>* elem%% = nullptr;",
          elem.tokType.typeInfo->name, elem.tokName.str);
        a("res = kSv.getElement(\"%%\", elem%%);",
          elem.tokName.str, elem.tokName.str);
        a("if (res != SUCCESS)");
        a("{");
        a.increaseIndent();
        a("return res;");
        a.decreaseIndent();
        a("}");
        a();
    }

    return SUCCESS;
}

// static Result codeExpression(const Ref<const ExpressionParse> kParse,
//                              const Ref<const StateMachineAssembly> kSmAsm,
//                              Autocode& kAutocode,
//                              Workspace& kWs,
//                              String& kExprAddr)
// {
//     SF_SAFE_ASSERT(kSmAsm != nullptr);

//     Autocode& a = kAutocode;

//     if (kParse == nullptr)
//     {
//         kExprAddr = "nullptr";
//         return SUCCESS;
//     }

//     if (kParse->func)
//     {
//         SF_ASSERT(false);
//     }

//     if (kParse->data.type == Token::CONSTANT)
//     {
//         // Constant node.
//         a("static ConstExprNode<F64> node%%(%%);",
//           kWs.exprNodeCnt, kParse->data.str);
//     }
//     else if (kParse->data.type == Token::IDENTIFIER)
//     {
//         // Identifier/state vector element node.

//         // Define node which evaluates element.
//         a("static ElementExprNode<F64> node%%(elem%%);",
//           kWs.exprNodeCnt, kParse->data.str);
//         ++kWs.exprNodeCnt;

//         // Define node which casts element to F64.
//         a("static UnaryOpExprNode<F64, X> node%%(safeCast<F64, X>, node%%);",
//           kWs.exprNodeCnt, (kWs.exprNodeCnt - 1));
//     }
//     else
//     {
//         // Operator node.
//     }

//     // Return address of defined node.
//     kExprAddr = Autocode::format("&node%%", kWs.exprNodeCnt);
//     ++kWs.exprNodeCnt;

//     return SUCCESS;
// }

static Result codeBlock(const Ref<const StateMachineParse::BlockParse> kParse,
                        const Ref<const StateMachineAssembly> kSmAsm,
                        Autocode& kAutocode,
                        Workspace& kWs,
                        String& kBlockAddr)
{
    SF_SAFE_ASSERT(kSmAsm != nullptr);

    // Base case: block is null, so represent as a null pointer in the owning
    // structure.
    if (kParse == nullptr)
    {
        kBlockAddr = "nullptr";
        return SUCCESS;
    }

    // Define guard expression if the block is guarded.
    String guardAddr = "nullptr";
    // TODO

    // Define if branch block.
    String ifBlockAddr;
    Result res = codeBlock(kParse->ifBlock,
                           kSmAsm,
                           kAutocode,
                           kWs,
                           ifBlockAddr);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Define else branch block.
    String elseBlockAddr;
    res = codeBlock(kParse->elseBlock, kSmAsm, kAutocode, kWs, elseBlockAddr);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Define action if block has one.
    String actionAddr = "nullptr";
    // TODO

    // Define next block.
    String nextBlockAddr;
    res = codeBlock(kParse->next, kSmAsm, kAutocode, kWs, nextBlockAddr);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Generate a unique identifier for the block and set the return string for
    // taking its address.
    Autocode& a = kAutocode;
    const String blockId = Autocode::format("block%%", kWs.blockCnt++);
    kBlockAddr = Autocode::format("&%%", blockId);

    // Define block.
    a("static StateMachine::Block %%{%%, %%, %%, %%, %%};",
      blockId,
      guardAddr,
      ifBlockAddr,
      elseBlockAddr,
      actionAddr,
      nextBlockAddr);

    return SUCCESS;
}

static Result codeState(const StateMachineParse::StateParse& kParse,
                        const Ref<const StateMachineAssembly> kSmAsm,
                        Autocode& kAutocode,
                        Workspace& kWs)
{
    SF_SAFE_ASSERT(kSmAsm != nullptr);

    Autocode& a = kAutocode;

    ++kWs.stateCnt;

    // Add comment at start of state config definition.
    const String& sectionName = kParse.tokName.str;
    SF_SAFE_ASSERT(kParse.tokName.str.size() >= 3);
    const String stateName = sectionName.substr(1, (sectionName.size() - 2));
    kWs.stateNames.push_back(stateName);
    a("// State `%%`", stateName);

    // Generate entry block code.
    String entryBlockAddr;
    Result res = codeBlock(kParse.entry, kSmAsm, a, kWs, entryBlockAddr);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Generate step block code.
    String stepBlockAddr;
    res = codeBlock(kParse.step, kSmAsm, a, kWs, stepBlockAddr);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Generate exit block code.
    String exitBlockAddr;
    res = codeBlock(kParse.exit, kSmAsm, a, kWs, exitBlockAddr);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Define state config.
    a("static const StateMachine::StateConfig state%%Config{%%, %%, %%, %%};",
      stateName, kWs.stateCnt, entryBlockAddr, stepBlockAddr, exitBlockAddr);

    return SUCCESS;
}

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

    const Ref<const StateMachineParse> parse = kSmAsm->parse();
    SF_SAFE_ASSERT(parse != nullptr);

    Autocode a(kOs);

    // Initialize a blank workspace for the autocoder.
    Workspace ws{};

    // Add preamble.
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
    a("static Result init(StateMachine& kSm, StateVector& kSv)");
    a("{");
    a.increaseIndent();

    a("Result res = SUCCESS;");
    a();

    // Define local state vector elements.
    Result res = codeLocalStateVector(parse, a);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Get state vector elements from state vector.
    res = codeStateVectorElems(parse, a);
    SF_DISABLE_CODE_COV;
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ENABLE_CODE_COV;

    // Define state configs.
    for (const StateMachineParse::StateParse& state : parse->states)
    {
        res = codeState(state, kSmAsm, a, ws);
        SF_DISABLE_CODE_COV;
        if (res != SUCCESS)
        {
            return res;
        }
        SF_DISABLE_CODE_COV;
        a();
    }

    // Define state config array.
    a("static const StateMachine::StateConfig stateConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const String& state : ws.stateNames)
    {
        a("state%%Config,", state);
    }

    a("{StateMachine::NO_STATE, nullptr, nullptr, nullptr}"); // Null terminator
    a.decreaseIndent();
    a("}");
    a();

    // Define expression stats array.
    String exprStatsArrAddr = "nullptr";
    if (ws.exprStatIds.size() > 0)
    {
        SF_ASSERT(false);
    }

    // Define state machine config.
    a("static const StateMachine::Config smConfig{%%, %%, %%, %%, %%};",
      "foo", "&elemT", "baz", "stateConfigs", exprStatsArrAddr);
    a();

    // Add return statement which initializes the state machine.
    a("return StateMachine::create(smConfig, kSm);");

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
