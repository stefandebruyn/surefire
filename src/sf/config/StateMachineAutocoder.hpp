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

    struct Workspace final
    {
        Ref<const StateMachineAssembly> smAsm;
        Set<const IElement*> refElems;
        U32 blockCnt;
        U32 exprNodeCnt;
        U32 stateCnt;
        U32 actCnt;
        U32 statsCnt;
    };

    static const Map<const void*, String> opFuncIds;

    static const Map<IExpression::NodeType, String> exprStatNodeIds;

    static String elemNameFromAddr(const IElement* const kAddr,
                                   StateMachineAutocoder::Workspace& kWs);

    static void codeLocalStateVector(Autocode& kAutocode,
                                     StateMachineAutocoder::Workspace& kWs);

    static String codeConstExprNode(const IExpression* const kNode,
                                    Autocode& kAutocode,
                                    StateMachineAutocoder::Workspace& kWs);

    static void codeElementLookup(Autocode& kAutocode,
                                  const IElement* const kElemObj,
                                  const TypeInfo& kElemTypeInfo,
                                  const String kElemName,
                                  StateMachineAutocoder::Workspace& kWs);

    static String codeElementExprNode(const IExpression* const kNode,
                                      Autocode& kAutocode,
                                      StateMachineAutocoder::Workspace& kWs);

    static String codeBinOpExprNode(const IExpression* const kNode,
                                    Autocode& kAutocode,
                                    StateMachineAutocoder::Workspace& kWs);

    static String codeUnaryOpExprNode(const IExpression* const kNode,
                                      Autocode& kAutocode,
                                      StateMachineAutocoder::Workspace& kWs);

    static String codeExprStatsNode(const IExpression* const kNode,
                                    Autocode& kAutocode,
                                    StateMachineAutocoder::Workspace& kWs);

    static String codeExpression(const IExpression* const kExpr,
                                 Autocode& kAutocode,
                                 StateMachineAutocoder::Workspace& kWs);

    static String codeAction(const IAction* const kAction,
                             Autocode& kAutocode,
                             StateMachineAutocoder::Workspace& kWs);

    static String codeBlock(const StateMachine::Block* const kBlock,
                            Autocode& kAutocode,
                            StateMachineAutocoder::Workspace& kWs);

    static void codeState(const StateMachine::StateConfig* const kState,
                          Autocode& kAutocode,
                          StateMachineAutocoder::Workspace& kWs);
};

#endif
