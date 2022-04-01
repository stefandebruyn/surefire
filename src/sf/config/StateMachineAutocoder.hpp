#ifndef SF_STATE_MACHINE_AUTOCODER_HPP
#define SF_STATE_MACHINE_AUTOCODER_HPP

#include "sf/config/Autocode.hpp"
#include "sf/config/StateMachineCompiler.hpp"

class StateMachineAutocoder final
{
public:

    static Result code(std::ostream& kOs,
                       const String kName,
                       const Ref<const StateMachineAssembly> kAsm);

    StateMachineAutocoder() = delete;

private:

    static Map<String, String> mFuncNodeTypeNames;

    struct Workspace final
    {
        U32 blockCnt;
        U32 exprNodeCnt;
        U32 stateCnt;
        U32 actCnt;
        U32 statsCnt;
        Vec<String> exprStatIds;
        Vec<String> stateNames;
        Map<String, const TypeInfo*> elemTypeInfo;
        Map<String, U32> stateIds;
        Map<String, String> elemTrueNames;
    };

    static void codeLocalStateVector(
        const Ref<const StateMachineParse> kParse,
        const Ref<const StateMachineAssembly> kSmAsm,
        Autocode& kAutocode,
        StateMachineAutocoder::Workspace& kWs);

    static void codeStateVectorElems(const Ref<const StateMachineParse> kParse,
                                     Autocode& kAutocode,
                                     StateMachineAutocoder::Workspace& kWs);

    static String codeFunctionCall(const Ref<const ExpressionParse> kParse,
                                   const Ref<const StateMachineAssembly> kSmAsm,
                                   Autocode& kAutocode,
                                   StateMachineAutocoder::Workspace& kWs);

    static String codeExpression(const Ref<const ExpressionParse> kParse,
                                 const Ref<const StateMachineAssembly> kSmAsm,
                                 const ElementType kEvalType,
                                 Autocode& kAutocode,
                                 StateMachineAutocoder::Workspace& kWs);

    static String codeAction(
        const Ref<const StateMachineParse::ActionParse> kParse,
        const Ref<const StateMachineAssembly> kSmAsm,
        Autocode& kAutocode,
        StateMachineAutocoder::Workspace& kWs);

    static String codeBlock(
        const Ref<const StateMachineParse::BlockParse> kParse,
        const Ref<const StateMachineAssembly> kSmAsm,
        Autocode& kAutocode,
        StateMachineAutocoder::Workspace& kWs);

    static void codeState(const StateMachineParse::StateParse& kParse,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Autocode& kAutocode,
                          StateMachineAutocoder::Workspace& kWs);
};

#endif
