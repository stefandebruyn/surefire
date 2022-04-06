///
/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.
///

#ifndef HalcyonStateMachine_HPP
#define HalcyonStateMachine_HPP

#include "sf/core/StateMachine.hpp"
#include "sf/core/StateVector.hpp"

namespace HalcyonStateMachine
{

static Result getConfig(StateVector& kSv, StateMachine::Config& kSmConfig)
{
    Result res = SUCCESS;

    // Local state vector
    static struct
    {
        U64 T = 0ULL;
        U8 MODE_SAFED = 0;
        U8 MODE_ENABLED = 1;
        U8 CMD_PROCEED = 4;
        F64 MIN_APOGEE_FOR_RECOVERY_M = 2400;
    } localBacking;

    static Element<U64> elemObjT(localBacking.T);
    static Element<U64>* elemT = &elemObjT;
    static Element<U8> elemObjMODE_SAFED(localBacking.MODE_SAFED);
    static Element<U8>* elemMODE_SAFED = &elemObjMODE_SAFED;
    static Element<U8> elemObjMODE_ENABLED(localBacking.MODE_ENABLED);
    static Element<U8>* elemMODE_ENABLED = &elemObjMODE_ENABLED;
    static Element<U8> elemObjCMD_PROCEED(localBacking.CMD_PROCEED);
    static Element<U8>* elemCMD_PROCEED = &elemObjCMD_PROCEED;
    static Element<F64> elemObjMIN_APOGEE_FOR_RECOVERY_M(localBacking.MIN_APOGEE_FOR_RECOVERY_M);
    static Element<F64>* elemMIN_APOGEE_FOR_RECOVERY_M = &elemObjMIN_APOGEE_FOR_RECOVERY_M;

    // State 1 config
    static ElementExprNode<U8> node2(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node1(ExprOpFuncs::safeCast<F64, U8>, *&node2);
    static UnaryOpExprNode<U8, F64> node0(ExprOpFuncs::safeCast<U8, F64>, *&node1);
    Element<U8>* elemDV_ELEM_FC_STARTUP_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_FC_STARTUP_CTRL_MODE", elemDV_ELEM_FC_STARTUP_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act0(*elemDV_ELEM_FC_STARTUP_CTRL_MODE, *&node0);
    static ElementExprNode<U8> node5(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node4(ExprOpFuncs::safeCast<F64, U8>, *&node5);
    static UnaryOpExprNode<U8, F64> node3(ExprOpFuncs::safeCast<U8, F64>, *&node4);
    Element<U8>* elemDV_ELEM_FC_HEALTH_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_FC_HEALTH_CTRL_MODE", elemDV_ELEM_FC_HEALTH_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act1(*elemDV_ELEM_FC_HEALTH_CTRL_MODE, *&node3);
    static ElementExprNode<U8> node8(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node7(ExprOpFuncs::safeCast<F64, U8>, *&node8);
    static UnaryOpExprNode<U8, F64> node6(ExprOpFuncs::safeCast<U8, F64>, *&node7);
    Element<U8>* elemDV_ELEM_FLUIDS_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_FLUIDS_CTRL_MODE", elemDV_ELEM_FLUIDS_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act2(*elemDV_ELEM_FLUIDS_CTRL_MODE, *&node6);
    static ElementExprNode<U8> node11(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node10(ExprOpFuncs::safeCast<F64, U8>, *&node11);
    static UnaryOpExprNode<U8, F64> node9(ExprOpFuncs::safeCast<U8, F64>, *&node10);
    Element<U8>* elemDV_ELEM_DN0_DESYNC_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN0_DESYNC_CTRL_MODE", elemDV_ELEM_DN0_DESYNC_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act3(*elemDV_ELEM_DN0_DESYNC_CTRL_MODE, *&node9);
    static ElementExprNode<U8> node14(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node13(ExprOpFuncs::safeCast<F64, U8>, *&node14);
    static UnaryOpExprNode<U8, F64> node12(ExprOpFuncs::safeCast<U8, F64>, *&node13);
    Element<U8>* elemDV_ELEM_DN1_DESYNC_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN1_DESYNC_CTRL_MODE", elemDV_ELEM_DN1_DESYNC_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act4(*elemDV_ELEM_DN1_DESYNC_CTRL_MODE, *&node12);
    static ElementExprNode<U8> node17(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node16(ExprOpFuncs::safeCast<F64, U8>, *&node17);
    static UnaryOpExprNode<U8, F64> node15(ExprOpFuncs::safeCast<U8, F64>, *&node16);
    Element<U8>* elemDV_ELEM_DN2_DESYNC_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN2_DESYNC_CTRL_MODE", elemDV_ELEM_DN2_DESYNC_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act5(*elemDV_ELEM_DN2_DESYNC_CTRL_MODE, *&node15);
    static ElementExprNode<U8> node20(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node19(ExprOpFuncs::safeCast<F64, U8>, *&node20);
    static UnaryOpExprNode<U8, F64> node18(ExprOpFuncs::safeCast<U8, F64>, *&node19);
    Element<U8>* elemDV_ELEM_ABORT_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_ABORT_CTRL_MODE", elemDV_ELEM_ABORT_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act6(*elemDV_ELEM_ABORT_CTRL_MODE, *&node18);
    static StateMachine::Block block6{nullptr, nullptr, nullptr, &act6, nullptr};
    static StateMachine::Block block5{nullptr, nullptr, nullptr, &act5, &block6};
    static StateMachine::Block block4{nullptr, nullptr, nullptr, &act4, &block5};
    static StateMachine::Block block3{nullptr, nullptr, nullptr, &act3, &block4};
    static StateMachine::Block block2{nullptr, nullptr, nullptr, &act2, &block3};
    static StateMachine::Block block1{nullptr, nullptr, nullptr, &act1, &block2};
    static StateMachine::Block block0{nullptr, nullptr, nullptr, &act0, &block1};
    Element<bool>* elemDV_ELEM_PROCEED_TO_FILL = nullptr;
    res = kSv.getElement("DV_ELEM_PROCEED_TO_FILL", elemDV_ELEM_PROCEED_TO_FILL);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node23(*elemDV_ELEM_PROCEED_TO_FILL);
    static UnaryOpExprNode<F64, bool> node22(ExprOpFuncs::safeCast<F64, bool>, *&node23);
    static UnaryOpExprNode<bool, F64> node21(ExprOpFuncs::safeCast<bool, F64>, *&node22);
    static TransitionAction act7(2);
    static StateMachine::Block block8{nullptr, nullptr, nullptr, &act7, nullptr};
    Element<bool>* elemDV_ELEM_ABORT = nullptr;
    res = kSv.getElement("DV_ELEM_ABORT", elemDV_ELEM_ABORT);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node26(*elemDV_ELEM_ABORT);
    static UnaryOpExprNode<F64, bool> node25(ExprOpFuncs::safeCast<F64, bool>, *&node26);
    static UnaryOpExprNode<bool, F64> node24(ExprOpFuncs::safeCast<bool, F64>, *&node25);
    static TransitionAction act8(3);
    static StateMachine::Block block10{nullptr, nullptr, nullptr, &act8, nullptr};
    static StateMachine::Block block9{&node24, &block10, nullptr, nullptr, nullptr};
    static StateMachine::Block block7{&node21, &block8, nullptr, nullptr, &block9};
    static StateMachine::StateConfig state1Config = {1, &block0, &block7, nullptr};

    // State 2 config
    static ElementExprNode<U8> node29(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node28(ExprOpFuncs::safeCast<F64, U8>, *&node29);
    static UnaryOpExprNode<U8, F64> node27(ExprOpFuncs::safeCast<U8, F64>, *&node28);
    static AssignmentAction<U8> act9(*elemDV_ELEM_FC_STARTUP_CTRL_MODE, *&node27);
    static ElementExprNode<U8> node32(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node31(ExprOpFuncs::safeCast<F64, U8>, *&node32);
    static UnaryOpExprNode<U8, F64> node30(ExprOpFuncs::safeCast<U8, F64>, *&node31);
    Element<U8>* elemDV_ELEM_CN_WATCHDOG_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_CN_WATCHDOG_CTRL_MODE", elemDV_ELEM_CN_WATCHDOG_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act10(*elemDV_ELEM_CN_WATCHDOG_CTRL_MODE, *&node30);
    static ElementExprNode<U8> node35(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node34(ExprOpFuncs::safeCast<F64, U8>, *&node35);
    static UnaryOpExprNode<U8, F64> node33(ExprOpFuncs::safeCast<U8, F64>, *&node34);
    Element<U8>* elemDV_ELEM_DN0_WATCHDOG_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN0_WATCHDOG_CTRL_MODE", elemDV_ELEM_DN0_WATCHDOG_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act11(*elemDV_ELEM_DN0_WATCHDOG_CTRL_MODE, *&node33);
    static ElementExprNode<U8> node38(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node37(ExprOpFuncs::safeCast<F64, U8>, *&node38);
    static UnaryOpExprNode<U8, F64> node36(ExprOpFuncs::safeCast<U8, F64>, *&node37);
    Element<U8>* elemDV_ELEM_DN1_WATCHDOG_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN1_WATCHDOG_CTRL_MODE", elemDV_ELEM_DN1_WATCHDOG_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act12(*elemDV_ELEM_DN1_WATCHDOG_CTRL_MODE, *&node36);
    static ElementExprNode<U8> node41(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node40(ExprOpFuncs::safeCast<F64, U8>, *&node41);
    static UnaryOpExprNode<U8, F64> node39(ExprOpFuncs::safeCast<U8, F64>, *&node40);
    Element<U8>* elemDV_ELEM_DN2_WATCHDOG_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN2_WATCHDOG_CTRL_MODE", elemDV_ELEM_DN2_WATCHDOG_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act13(*elemDV_ELEM_DN2_WATCHDOG_CTRL_MODE, *&node39);
    static ElementExprNode<U8> node44(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node43(ExprOpFuncs::safeCast<F64, U8>, *&node44);
    static UnaryOpExprNode<U8, F64> node42(ExprOpFuncs::safeCast<U8, F64>, *&node43);
    Element<U8>* elemDV_ELEM_GNC_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_GNC_CTRL_MODE", elemDV_ELEM_GNC_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act14(*elemDV_ELEM_GNC_CTRL_MODE, *&node42);
    static StateMachine::Block block16{nullptr, nullptr, nullptr, &act14, nullptr};
    static StateMachine::Block block15{nullptr, nullptr, nullptr, &act13, &block16};
    static StateMachine::Block block14{nullptr, nullptr, nullptr, &act12, &block15};
    static StateMachine::Block block13{nullptr, nullptr, nullptr, &act11, &block14};
    static StateMachine::Block block12{nullptr, nullptr, nullptr, &act10, &block13};
    static StateMachine::Block block11{nullptr, nullptr, nullptr, &act9, &block12};
    Element<bool>* elemDV_ELEM_PROCEED_TO_TERM_CNT = nullptr;
    res = kSv.getElement("DV_ELEM_PROCEED_TO_TERM_CNT", elemDV_ELEM_PROCEED_TO_TERM_CNT);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node47(*elemDV_ELEM_PROCEED_TO_TERM_CNT);
    static UnaryOpExprNode<F64, bool> node46(ExprOpFuncs::safeCast<F64, bool>, *&node47);
    static UnaryOpExprNode<bool, F64> node45(ExprOpFuncs::safeCast<bool, F64>, *&node46);
    static TransitionAction act15(5);
    static StateMachine::Block block18{nullptr, nullptr, nullptr, &act15, nullptr};
    static ElementExprNode<bool> node50(*elemDV_ELEM_ABORT);
    static UnaryOpExprNode<F64, bool> node49(ExprOpFuncs::safeCast<F64, bool>, *&node50);
    static UnaryOpExprNode<bool, F64> node48(ExprOpFuncs::safeCast<bool, F64>, *&node49);
    static TransitionAction act16(3);
    static StateMachine::Block block20{nullptr, nullptr, nullptr, &act16, nullptr};
    static StateMachine::Block block19{&node48, &block20, nullptr, nullptr, nullptr};
    static StateMachine::Block block17{&node45, &block18, nullptr, nullptr, &block19};
    static StateMachine::StateConfig state2Config = {2, &block11, &block17, nullptr};

    // State 3 config
    static ConstExprNode<F64> node52(0);
    static UnaryOpExprNode<bool, F64> node51(ExprOpFuncs::safeCast<bool, F64>, *&node52);
    static AssignmentAction<bool> act17(*elemDV_ELEM_PROCEED_TO_FILL, *&node51);
    static ConstExprNode<F64> node54(0);
    static UnaryOpExprNode<bool, F64> node53(ExprOpFuncs::safeCast<bool, F64>, *&node54);
    static AssignmentAction<bool> act18(*elemDV_ELEM_PROCEED_TO_TERM_CNT, *&node53);
    static ConstExprNode<F64> node56(0);
    static UnaryOpExprNode<bool, F64> node55(ExprOpFuncs::safeCast<bool, F64>, *&node56);
    static AssignmentAction<bool> act19(*elemDV_ELEM_ABORT, *&node55);
    static ConstExprNode<F64> node58(0);
    static UnaryOpExprNode<bool, F64> node57(ExprOpFuncs::safeCast<bool, F64>, *&node58);
    Element<bool>* elemDV_ELEM_BURN_BABY_BURN = nullptr;
    res = kSv.getElement("DV_ELEM_BURN_BABY_BURN", elemDV_ELEM_BURN_BABY_BURN);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<bool> act20(*elemDV_ELEM_BURN_BABY_BURN, *&node57);
    static ElementExprNode<U8> node61(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node60(ExprOpFuncs::safeCast<F64, U8>, *&node61);
    static UnaryOpExprNode<U8, F64> node59(ExprOpFuncs::safeCast<U8, F64>, *&node60);
    Element<U8>* elemDV_ELEM_CN_LOGGING_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_CN_LOGGING_CTRL_MODE", elemDV_ELEM_CN_LOGGING_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act21(*elemDV_ELEM_CN_LOGGING_CTRL_MODE, *&node59);
    static ElementExprNode<U8> node64(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node63(ExprOpFuncs::safeCast<F64, U8>, *&node64);
    static UnaryOpExprNode<U8, F64> node62(ExprOpFuncs::safeCast<U8, F64>, *&node63);
    Element<U8>* elemDV_ELEM_DN0_LOGGING_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN0_LOGGING_CTRL_MODE", elemDV_ELEM_DN0_LOGGING_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act22(*elemDV_ELEM_DN0_LOGGING_CTRL_MODE, *&node62);
    static ElementExprNode<U8> node67(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node66(ExprOpFuncs::safeCast<F64, U8>, *&node67);
    static UnaryOpExprNode<U8, F64> node65(ExprOpFuncs::safeCast<U8, F64>, *&node66);
    Element<U8>* elemDV_ELEM_DN1_LOGGING_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN1_LOGGING_CTRL_MODE", elemDV_ELEM_DN1_LOGGING_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act23(*elemDV_ELEM_DN1_LOGGING_CTRL_MODE, *&node65);
    static ElementExprNode<U8> node70(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node69(ExprOpFuncs::safeCast<F64, U8>, *&node70);
    static UnaryOpExprNode<U8, F64> node68(ExprOpFuncs::safeCast<U8, F64>, *&node69);
    Element<U8>* elemDV_ELEM_DN2_LOGGING_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_DN2_LOGGING_CTRL_MODE", elemDV_ELEM_DN2_LOGGING_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act24(*elemDV_ELEM_DN2_LOGGING_CTRL_MODE, *&node68);
    static StateMachine::Block block28{nullptr, nullptr, nullptr, &act24, nullptr};
    static StateMachine::Block block27{nullptr, nullptr, nullptr, &act23, &block28};
    static StateMachine::Block block26{nullptr, nullptr, nullptr, &act22, &block27};
    static StateMachine::Block block25{nullptr, nullptr, nullptr, &act21, &block26};
    static StateMachine::Block block24{nullptr, nullptr, nullptr, &act20, &block25};
    static StateMachine::Block block23{nullptr, nullptr, nullptr, &act19, &block24};
    static StateMachine::Block block22{nullptr, nullptr, nullptr, &act18, &block23};
    static StateMachine::Block block21{nullptr, nullptr, nullptr, &act17, &block22};
    Element<U8>* elemDV_ELEM_CMD = nullptr;
    res = kSv.getElement("DV_ELEM_CMD", elemDV_ELEM_CMD);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<U8> node74(*elemDV_ELEM_CMD);
    static UnaryOpExprNode<F64, U8> node73(ExprOpFuncs::safeCast<F64, U8>, *&node74);
    static ElementExprNode<U8> node76(*elemCMD_PROCEED);
    static UnaryOpExprNode<F64, U8> node75(ExprOpFuncs::safeCast<F64, U8>, *&node76);
    static BinOpExprNode<F64, F64> node72(ExprOpFuncs::eq<F64>, *&node73, *&node75);
    static UnaryOpExprNode<bool, F64> node71(ExprOpFuncs::safeCast<bool, F64>, *&node72);
    static TransitionAction act25(2);
    static StateMachine::Block block30{nullptr, nullptr, nullptr, &act25, nullptr};
    static ElementExprNode<bool> node79(*elemDV_ELEM_ABORT);
    static UnaryOpExprNode<F64, bool> node78(ExprOpFuncs::safeCast<F64, bool>, *&node79);
    static UnaryOpExprNode<bool, F64> node77(ExprOpFuncs::safeCast<bool, F64>, *&node78);
    static TransitionAction act26(4);
    static StateMachine::Block block32{nullptr, nullptr, nullptr, &act26, nullptr};
    static StateMachine::Block block31{&node77, &block32, nullptr, nullptr, nullptr};
    static StateMachine::Block block29{&node71, &block30, nullptr, nullptr, &block31};
    static StateMachine::StateConfig state3Config = {3, &block21, &block29, nullptr};

    // State 4 config
    static ElementExprNode<U8> node83(*elemDV_ELEM_CMD);
    static UnaryOpExprNode<F64, U8> node82(ExprOpFuncs::safeCast<F64, U8>, *&node83);
    static ElementExprNode<U8> node85(*elemCMD_PROCEED);
    static UnaryOpExprNode<F64, U8> node84(ExprOpFuncs::safeCast<F64, U8>, *&node85);
    static BinOpExprNode<F64, F64> node81(ExprOpFuncs::eq<F64>, *&node82, *&node84);
    static UnaryOpExprNode<bool, F64> node80(ExprOpFuncs::safeCast<bool, F64>, *&node81);
    static TransitionAction act27(3);
    static StateMachine::Block block34{nullptr, nullptr, nullptr, &act27, nullptr};
    static StateMachine::Block block33{&node80, &block34, nullptr, nullptr, nullptr};
    static StateMachine::StateConfig state4Config = {4, nullptr, &block33, nullptr};

    // State 5 config
    Element<bool>* elemDV_ELEM_PROCEED_TO_LAUNCH = nullptr;
    res = kSv.getElement("DV_ELEM_PROCEED_TO_LAUNCH", elemDV_ELEM_PROCEED_TO_LAUNCH);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node88(*elemDV_ELEM_PROCEED_TO_LAUNCH);
    static UnaryOpExprNode<F64, bool> node87(ExprOpFuncs::safeCast<F64, bool>, *&node88);
    static UnaryOpExprNode<bool, F64> node86(ExprOpFuncs::safeCast<bool, F64>, *&node87);
    static TransitionAction act28(6);
    static StateMachine::Block block36{nullptr, nullptr, nullptr, &act28, nullptr};
    static ElementExprNode<bool> node91(*elemDV_ELEM_ABORT);
    static UnaryOpExprNode<F64, bool> node90(ExprOpFuncs::safeCast<F64, bool>, *&node91);
    static UnaryOpExprNode<bool, F64> node89(ExprOpFuncs::safeCast<bool, F64>, *&node90);
    static TransitionAction act29(3);
    static StateMachine::Block block38{nullptr, nullptr, nullptr, &act29, nullptr};
    static StateMachine::Block block37{&node89, &block38, nullptr, nullptr, nullptr};
    static StateMachine::Block block35{&node86, &block36, nullptr, nullptr, &block37};
    static StateMachine::StateConfig state5Config = {5, nullptr, &block35, nullptr};

    // State 6 config
    static ElementExprNode<U8> node94(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node93(ExprOpFuncs::safeCast<F64, U8>, *&node94);
    static UnaryOpExprNode<U8, F64> node92(ExprOpFuncs::safeCast<U8, F64>, *&node93);
    static AssignmentAction<U8> act30(*elemDV_ELEM_CN_LOGGING_CTRL_MODE, *&node92);
    static ElementExprNode<U8> node97(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node96(ExprOpFuncs::safeCast<F64, U8>, *&node97);
    static UnaryOpExprNode<U8, F64> node95(ExprOpFuncs::safeCast<U8, F64>, *&node96);
    static AssignmentAction<U8> act31(*elemDV_ELEM_DN0_LOGGING_CTRL_MODE, *&node95);
    static ElementExprNode<U8> node100(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node99(ExprOpFuncs::safeCast<F64, U8>, *&node100);
    static UnaryOpExprNode<U8, F64> node98(ExprOpFuncs::safeCast<U8, F64>, *&node99);
    static AssignmentAction<U8> act32(*elemDV_ELEM_DN1_LOGGING_CTRL_MODE, *&node98);
    static ElementExprNode<U8> node103(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node102(ExprOpFuncs::safeCast<F64, U8>, *&node103);
    static UnaryOpExprNode<U8, F64> node101(ExprOpFuncs::safeCast<U8, F64>, *&node102);
    static AssignmentAction<U8> act33(*elemDV_ELEM_DN2_LOGGING_CTRL_MODE, *&node101);
    static StateMachine::Block block42{nullptr, nullptr, nullptr, &act33, nullptr};
    static StateMachine::Block block41{nullptr, nullptr, nullptr, &act32, &block42};
    static StateMachine::Block block40{nullptr, nullptr, nullptr, &act31, &block41};
    static StateMachine::Block block39{nullptr, nullptr, nullptr, &act30, &block40};
    Element<bool>* elemDV_ELEM_LIFTOFF_CONFIRMED = nullptr;
    res = kSv.getElement("DV_ELEM_LIFTOFF_CONFIRMED", elemDV_ELEM_LIFTOFF_CONFIRMED);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node106(*elemDV_ELEM_LIFTOFF_CONFIRMED);
    static UnaryOpExprNode<F64, bool> node105(ExprOpFuncs::safeCast<F64, bool>, *&node106);
    static UnaryOpExprNode<bool, F64> node104(ExprOpFuncs::safeCast<bool, F64>, *&node105);
    static TransitionAction act34(7);
    static StateMachine::Block block44{nullptr, nullptr, nullptr, &act34, nullptr};
    static ElementExprNode<bool> node109(*elemDV_ELEM_ABORT);
    static UnaryOpExprNode<F64, bool> node108(ExprOpFuncs::safeCast<F64, bool>, *&node109);
    static UnaryOpExprNode<bool, F64> node107(ExprOpFuncs::safeCast<bool, F64>, *&node108);
    static TransitionAction act35(3);
    static StateMachine::Block block46{nullptr, nullptr, nullptr, &act35, nullptr};
    static StateMachine::Block block45{&node107, &block46, nullptr, nullptr, nullptr};
    static StateMachine::Block block43{&node104, &block44, nullptr, nullptr, &block45};
    static StateMachine::StateConfig state6Config = {6, &block39, &block43, nullptr};

    // State 7 config
    static ElementExprNode<U8> node112(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node111(ExprOpFuncs::safeCast<F64, U8>, *&node112);
    static UnaryOpExprNode<U8, F64> node110(ExprOpFuncs::safeCast<U8, F64>, *&node111);
    Element<U8>* elemDV_ELEM_NCONE_CTRL_MODE_PRI = nullptr;
    res = kSv.getElement("DV_ELEM_NCONE_CTRL_MODE_PRI", elemDV_ELEM_NCONE_CTRL_MODE_PRI);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act36(*elemDV_ELEM_NCONE_CTRL_MODE_PRI, *&node110);
    static ElementExprNode<U8> node115(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node114(ExprOpFuncs::safeCast<F64, U8>, *&node115);
    static UnaryOpExprNode<U8, F64> node113(ExprOpFuncs::safeCast<U8, F64>, *&node114);
    Element<U8>* elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI = nullptr;
    res = kSv.getElement("DV_ELEM_DROGP_DEP_CTRL_MODE_PRI", elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act37(*elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI, *&node113);
    static ElementExprNode<U8> node118(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node117(ExprOpFuncs::safeCast<F64, U8>, *&node118);
    static UnaryOpExprNode<U8, F64> node116(ExprOpFuncs::safeCast<U8, F64>, *&node117);
    Element<U8>* elemDV_ELEM_MAINP_CTRL_MODE_PRI = nullptr;
    res = kSv.getElement("DV_ELEM_MAINP_CTRL_MODE_PRI", elemDV_ELEM_MAINP_CTRL_MODE_PRI);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act38(*elemDV_ELEM_MAINP_CTRL_MODE_PRI, *&node116);
    static ElementExprNode<U8> node121(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node120(ExprOpFuncs::safeCast<F64, U8>, *&node121);
    static UnaryOpExprNode<U8, F64> node119(ExprOpFuncs::safeCast<U8, F64>, *&node120);
    Element<U8>* elemDV_ELEM_NCONE_CTRL_MODE_BAK = nullptr;
    res = kSv.getElement("DV_ELEM_NCONE_CTRL_MODE_BAK", elemDV_ELEM_NCONE_CTRL_MODE_BAK);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act39(*elemDV_ELEM_NCONE_CTRL_MODE_BAK, *&node119);
    static ElementExprNode<U8> node124(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node123(ExprOpFuncs::safeCast<F64, U8>, *&node124);
    static UnaryOpExprNode<U8, F64> node122(ExprOpFuncs::safeCast<U8, F64>, *&node123);
    Element<U8>* elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK = nullptr;
    res = kSv.getElement("DV_ELEM_DROGP_DEP_CTRL_MODE_BAK", elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act40(*elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK, *&node122);
    static ElementExprNode<U8> node127(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node126(ExprOpFuncs::safeCast<F64, U8>, *&node127);
    static UnaryOpExprNode<U8, F64> node125(ExprOpFuncs::safeCast<U8, F64>, *&node126);
    Element<U8>* elemDV_ELEM_MAINP_CTRL_MODE_BAK = nullptr;
    res = kSv.getElement("DV_ELEM_MAINP_CTRL_MODE_BAK", elemDV_ELEM_MAINP_CTRL_MODE_BAK);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act41(*elemDV_ELEM_MAINP_CTRL_MODE_BAK, *&node125);
    static ElementExprNode<U8> node130(*elemMODE_ENABLED);
    static UnaryOpExprNode<F64, U8> node129(ExprOpFuncs::safeCast<F64, U8>, *&node130);
    static UnaryOpExprNode<U8, F64> node128(ExprOpFuncs::safeCast<U8, F64>, *&node129);
    Element<U8>* elemDV_ELEM_MTIME_CTRL_MODE = nullptr;
    res = kSv.getElement("DV_ELEM_MTIME_CTRL_MODE", elemDV_ELEM_MTIME_CTRL_MODE);
    if (res != SUCCESS)
    {
        return res;
    }
    static AssignmentAction<U8> act42(*elemDV_ELEM_MTIME_CTRL_MODE, *&node128);
    static StateMachine::Block block53{nullptr, nullptr, nullptr, &act42, nullptr};
    static StateMachine::Block block52{nullptr, nullptr, nullptr, &act41, &block53};
    static StateMachine::Block block51{nullptr, nullptr, nullptr, &act40, &block52};
    static StateMachine::Block block50{nullptr, nullptr, nullptr, &act39, &block51};
    static StateMachine::Block block49{nullptr, nullptr, nullptr, &act38, &block50};
    static StateMachine::Block block48{nullptr, nullptr, nullptr, &act37, &block49};
    static StateMachine::Block block47{nullptr, nullptr, nullptr, &act36, &block48};
    static ElementExprNode<bool> node134(*elemDV_ELEM_BURN_BABY_BURN);
    static UnaryOpExprNode<F64, bool> node133(ExprOpFuncs::safeCast<F64, bool>, *&node134);
    static UnaryOpExprNode<F64, F64> node132(ExprOpFuncs::lnot<F64>, *&node133);
    static UnaryOpExprNode<bool, F64> node131(ExprOpFuncs::safeCast<bool, F64>, *&node132);
    static TransitionAction act43(8);
    static StateMachine::Block block55{nullptr, nullptr, nullptr, &act43, nullptr};
    static StateMachine::Block block54{&node131, &block55, nullptr, nullptr, nullptr};
    static StateMachine::StateConfig state7Config = {7, &block47, &block54, nullptr};

    // State 8 config
    Element<bool>* elemDV_ELEM_APOGEE = nullptr;
    res = kSv.getElement("DV_ELEM_APOGEE", elemDV_ELEM_APOGEE);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node137(*elemDV_ELEM_APOGEE);
    static UnaryOpExprNode<F64, bool> node136(ExprOpFuncs::safeCast<F64, bool>, *&node137);
    static UnaryOpExprNode<bool, F64> node135(ExprOpFuncs::safeCast<bool, F64>, *&node136);
    static TransitionAction act44(9);
    static StateMachine::Block block57{nullptr, nullptr, nullptr, &act44, nullptr};
    static StateMachine::Block block56{&node135, &block57, nullptr, nullptr, nullptr};
    static StateMachine::StateConfig state8Config = {8, nullptr, &block56, nullptr};

    // State 9 config
    Element<F64>* elemDV_ELEM_ROCKET_POS_X = nullptr;
    res = kSv.getElement("DV_ELEM_ROCKET_POS_X", elemDV_ELEM_ROCKET_POS_X);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<F64> node141(*elemDV_ELEM_ROCKET_POS_X);
    static UnaryOpExprNode<F64, F64> node140(ExprOpFuncs::safeCast<F64, F64>, *&node141);
    static ElementExprNode<F64> node143(*elemMIN_APOGEE_FOR_RECOVERY_M);
    static UnaryOpExprNode<F64, F64> node142(ExprOpFuncs::safeCast<F64, F64>, *&node143);
    static BinOpExprNode<F64, F64> node139(ExprOpFuncs::lt<F64>, *&node140, *&node142);
    static UnaryOpExprNode<bool, F64> node138(ExprOpFuncs::safeCast<bool, F64>, *&node139);
    static TransitionAction act45(10);
    static StateMachine::Block block59{nullptr, nullptr, nullptr, &act45, nullptr};
    static ElementExprNode<F64> node147(*elemDV_ELEM_ROCKET_POS_X);
    static UnaryOpExprNode<F64, F64> node146(ExprOpFuncs::safeCast<F64, F64>, *&node147);
    static ElementExprNode<F64> node149(*elemMIN_APOGEE_FOR_RECOVERY_M);
    static UnaryOpExprNode<F64, F64> node148(ExprOpFuncs::safeCast<F64, F64>, *&node149);
    static BinOpExprNode<F64, F64> node145(ExprOpFuncs::gte<F64>, *&node146, *&node148);
    static UnaryOpExprNode<bool, F64> node144(ExprOpFuncs::safeCast<bool, F64>, *&node145);
    static TransitionAction act46(11);
    static StateMachine::Block block61{nullptr, nullptr, nullptr, &act46, nullptr};
    static StateMachine::Block block60{&node144, &block61, nullptr, nullptr, nullptr};
    static StateMachine::Block block58{&node138, &block59, nullptr, nullptr, &block60};
    static StateMachine::StateConfig state9Config = {9, nullptr, &block58, nullptr};

    // State 10 config
    static ElementExprNode<U8> node152(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node151(ExprOpFuncs::safeCast<F64, U8>, *&node152);
    static UnaryOpExprNode<U8, F64> node150(ExprOpFuncs::safeCast<U8, F64>, *&node151);
    static AssignmentAction<U8> act47(*elemDV_ELEM_GNC_CTRL_MODE, *&node150);
    static ElementExprNode<U8> node155(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node154(ExprOpFuncs::safeCast<F64, U8>, *&node155);
    static UnaryOpExprNode<U8, F64> node153(ExprOpFuncs::safeCast<U8, F64>, *&node154);
    static AssignmentAction<U8> act48(*elemDV_ELEM_NCONE_CTRL_MODE_PRI, *&node153);
    static ElementExprNode<U8> node158(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node157(ExprOpFuncs::safeCast<F64, U8>, *&node158);
    static UnaryOpExprNode<U8, F64> node156(ExprOpFuncs::safeCast<U8, F64>, *&node157);
    static AssignmentAction<U8> act49(*elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI, *&node156);
    static ElementExprNode<U8> node161(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node160(ExprOpFuncs::safeCast<F64, U8>, *&node161);
    static UnaryOpExprNode<U8, F64> node159(ExprOpFuncs::safeCast<U8, F64>, *&node160);
    static AssignmentAction<U8> act50(*elemDV_ELEM_MAINP_CTRL_MODE_PRI, *&node159);
    static ElementExprNode<U8> node164(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node163(ExprOpFuncs::safeCast<F64, U8>, *&node164);
    static UnaryOpExprNode<U8, F64> node162(ExprOpFuncs::safeCast<U8, F64>, *&node163);
    static AssignmentAction<U8> act51(*elemDV_ELEM_NCONE_CTRL_MODE_BAK, *&node162);
    static ElementExprNode<U8> node167(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node166(ExprOpFuncs::safeCast<F64, U8>, *&node167);
    static UnaryOpExprNode<U8, F64> node165(ExprOpFuncs::safeCast<U8, F64>, *&node166);
    static AssignmentAction<U8> act52(*elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK, *&node165);
    static ElementExprNode<U8> node170(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node169(ExprOpFuncs::safeCast<F64, U8>, *&node170);
    static UnaryOpExprNode<U8, F64> node168(ExprOpFuncs::safeCast<U8, F64>, *&node169);
    static AssignmentAction<U8> act53(*elemDV_ELEM_MAINP_CTRL_MODE_BAK, *&node168);
    static StateMachine::Block block68{nullptr, nullptr, nullptr, &act53, nullptr};
    static StateMachine::Block block67{nullptr, nullptr, nullptr, &act52, &block68};
    static StateMachine::Block block66{nullptr, nullptr, nullptr, &act51, &block67};
    static StateMachine::Block block65{nullptr, nullptr, nullptr, &act50, &block66};
    static StateMachine::Block block64{nullptr, nullptr, nullptr, &act49, &block65};
    static StateMachine::Block block63{nullptr, nullptr, nullptr, &act48, &block64};
    static StateMachine::Block block62{nullptr, nullptr, nullptr, &act47, &block63};
    static StateMachine::StateConfig state10Config = {10, &block62, nullptr, nullptr};

    // State 11 config
    Element<bool>* elemDV_ELEM_TOUCHDOWN = nullptr;
    res = kSv.getElement("DV_ELEM_TOUCHDOWN", elemDV_ELEM_TOUCHDOWN);
    if (res != SUCCESS)
    {
        return res;
    }
    static ElementExprNode<bool> node173(*elemDV_ELEM_TOUCHDOWN);
    static UnaryOpExprNode<F64, bool> node172(ExprOpFuncs::safeCast<F64, bool>, *&node173);
    static UnaryOpExprNode<bool, F64> node171(ExprOpFuncs::safeCast<bool, F64>, *&node172);
    static TransitionAction act54(12);
    static StateMachine::Block block70{nullptr, nullptr, nullptr, &act54, nullptr};
    static StateMachine::Block block69{&node171, &block70, nullptr, nullptr, nullptr};
    static StateMachine::StateConfig state11Config = {11, nullptr, &block69, nullptr};

    // State 12 config
    static ElementExprNode<U8> node176(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node175(ExprOpFuncs::safeCast<F64, U8>, *&node176);
    static UnaryOpExprNode<U8, F64> node174(ExprOpFuncs::safeCast<U8, F64>, *&node175);
    static AssignmentAction<U8> act55(*elemDV_ELEM_NCONE_CTRL_MODE_PRI, *&node174);
    static ElementExprNode<U8> node179(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node178(ExprOpFuncs::safeCast<F64, U8>, *&node179);
    static UnaryOpExprNode<U8, F64> node177(ExprOpFuncs::safeCast<U8, F64>, *&node178);
    static AssignmentAction<U8> act56(*elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI, *&node177);
    static ElementExprNode<U8> node182(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node181(ExprOpFuncs::safeCast<F64, U8>, *&node182);
    static UnaryOpExprNode<U8, F64> node180(ExprOpFuncs::safeCast<U8, F64>, *&node181);
    static AssignmentAction<U8> act57(*elemDV_ELEM_MAINP_CTRL_MODE_PRI, *&node180);
    static ElementExprNode<U8> node185(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node184(ExprOpFuncs::safeCast<F64, U8>, *&node185);
    static UnaryOpExprNode<U8, F64> node183(ExprOpFuncs::safeCast<U8, F64>, *&node184);
    static AssignmentAction<U8> act58(*elemDV_ELEM_NCONE_CTRL_MODE_BAK, *&node183);
    static ElementExprNode<U8> node188(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node187(ExprOpFuncs::safeCast<F64, U8>, *&node188);
    static UnaryOpExprNode<U8, F64> node186(ExprOpFuncs::safeCast<U8, F64>, *&node187);
    static AssignmentAction<U8> act59(*elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK, *&node186);
    static ElementExprNode<U8> node191(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node190(ExprOpFuncs::safeCast<F64, U8>, *&node191);
    static UnaryOpExprNode<U8, F64> node189(ExprOpFuncs::safeCast<U8, F64>, *&node190);
    static AssignmentAction<U8> act60(*elemDV_ELEM_MAINP_CTRL_MODE_BAK, *&node189);
    static ElementExprNode<U8> node194(*elemMODE_SAFED);
    static UnaryOpExprNode<F64, U8> node193(ExprOpFuncs::safeCast<F64, U8>, *&node194);
    static UnaryOpExprNode<U8, F64> node192(ExprOpFuncs::safeCast<U8, F64>, *&node193);
    static AssignmentAction<U8> act61(*elemDV_ELEM_MTIME_CTRL_MODE, *&node192);
    static StateMachine::Block block77{nullptr, nullptr, nullptr, &act61, nullptr};
    static StateMachine::Block block76{nullptr, nullptr, nullptr, &act60, &block77};
    static StateMachine::Block block75{nullptr, nullptr, nullptr, &act59, &block76};
    static StateMachine::Block block74{nullptr, nullptr, nullptr, &act58, &block75};
    static StateMachine::Block block73{nullptr, nullptr, nullptr, &act57, &block74};
    static StateMachine::Block block72{nullptr, nullptr, nullptr, &act56, &block73};
    static StateMachine::Block block71{nullptr, nullptr, nullptr, &act55, &block72};
    static StateMachine::StateConfig state12Config = {12, &block71, nullptr, nullptr};

    // State machine config
    static StateMachine::StateConfig stateConfigs[] =
    {
        state1Config,
        state2Config,
        state3Config,
        state4Config,
        state5Config,
        state6Config,
        state7Config,
        state8Config,
        state9Config,
        state10Config,
        state11Config,
        state12Config,
        {StateMachine::NO_STATE, nullptr, nullptr, nullptr}
    };

    Element<U32>* elemDV_ELEM_STATE = nullptr;
    res = kSv.getElement("DV_ELEM_STATE", elemDV_ELEM_STATE);
    if (res != SUCCESS)
    {
        return res;
    }
    Element<U64>* elemDV_ELEM_CN_TIME = nullptr;
    res = kSv.getElement("DV_ELEM_CN_TIME", elemDV_ELEM_CN_TIME);
    if (res != SUCCESS)
    {
        return res;
    }

    static StateMachine::Config smConfig = {elemDV_ELEM_STATE, elemT, elemDV_ELEM_CN_TIME, stateConfigs, nullptr};
    kSmConfig = smConfig;

    return SUCCESS;
}

} // namespace HalcyonStateMachine

#endif
