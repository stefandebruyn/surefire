#include <cstring>

#include "sfa/sm/StateMachine.hpp"
#include "sfa/sm/TransitionAction.hpp"
#include "sfa/sm/AssignmentAction.hpp"
#include "sfa/sv/StateVector.hpp"
#include "utest/UTest.hpp"

/**************************** STATE VECTOR CONFIG *****************************/

#pragma pack(push, 1)
static struct
{
    U32 foo;
    F64 bar;
    bool baz;
    I32 qux;
} gSvBacking;
#pragma pack(pop)

static Element<U32> gElemFoo(gSvBacking.foo);
static Element<F64> gElemBar(gSvBacking.bar);
static Element<bool> gElemBaz(gSvBacking.baz);
static Element<I32> gElemQux(gSvBacking.qux);

/*************************** STATE MACHINE CONFIG *****************************/

// [STATE/State1]
// ENTRY:
//     qux = 1400
// STEP:
//     baz = true
//     (qux == 200 AND bar < 0.0): -> State2
//     bar = 9.81
// T[100, 200]:
//     bar = 7.777
//     baz = false
// EXIT:
//     bar = 1.522
static ExpressionTree<I32> gExpr1400(1400);
static ExpressionTree<F64> gExpr9p81(9.81);
static ExpressionTree<bool> gExprTrue(true);
static ExpressionTree<I32> gExpr200(200);
static ExpressionTree<F64> gExpr0p0(0.0);
static ExpressionTree<F64> gExpr7p777(7.777);
static ExpressionTree<bool> gExprFalse(false);
static ExpressionTree<F64> gExpr1p522(1.522);

static ExpressionTree<F64> gExprBar(gElemBar);
static ExpressionTree<bool> gExprBaz(gElemBaz);
static ExpressionTree<I32> gExprQux(gElemQux);

static ExpressionTree<bool, I32> gExprQuxEquals200(OP_EQUALS,
                                                   &gExprQux,
                                                   &gExpr200);
static ExpressionTree<bool, F64> gExprBarEquals0p0(OP_LESS_THAN,
                                                   &gExprBar,
                                                   &gExpr0p0);
static ExpressionTree<bool> gGuardTransState2(OP_AND,
                                              &gExprQuxEquals200,
                                              &gExprBarEquals0p0);

static AssignmentAction<I32> gActQuxGets1400(nullptr, gElemQux, gExpr1400);
static AssignmentAction<F64> gActBarGets9p81(nullptr, gElemBar, gExpr9p81);
static AssignmentAction<bool> gActBazGetsTrue(nullptr, gElemBaz, gExprTrue);
static TransitionAction gActTransState2(&gGuardTransState2, 2);
static AssignmentAction<F64> gActBarGets7p777(nullptr, gElemBar, gExpr7p777);
static AssignmentAction<bool> gActBazGetsFalse(nullptr, gElemBaz, gExprFalse);
static AssignmentAction<F64> gActBarGets1p522(nullptr, gElemBar, gExpr1p522);

static IAction* gState1EntryActs[] =
{
    &gActQuxGets1400,
    nullptr
};
static IAction* gState1StepActs[] =
{
    &gActBazGetsTrue,
    &gActTransState2,
    &gActBarGets9p81,
    nullptr
};
static IAction* gState1RangeActs[] =
{
    &gActBarGets7p777,
    &gActBazGetsFalse,
    nullptr
};
static IAction* gState1ExitActs[] =
{
    &gActBarGets1p522,
    nullptr
};
static StateMachine::LabelConfig gState1RangeLabels[] =
{
    {gState1RangeActs, 100, 200},
    {}
};
static StateMachine::StateConfig gState1Config =
{
    1,
    {gState1EntryActs},
    {gState1StepActs},
    gState1RangeLabels,
    {gState1ExitActs}
};

// [STATE/State2]
// ENTRY:
//     qux = 343
// STEP:
//     bar = 1.62
//     (baz == false): -> State1
// EXIT:
//     qux = 97
static ExpressionTree<I32> gExpr343(343);
static ExpressionTree<F64> gExpr1p62(1.62);
static ExpressionTree<I32> gExpr97(97);

static ExpressionTree<bool> gExprBazEqualsFalse(OP_EQUALS,
                                                &gExprBaz,
                                                &gExprFalse);

static AssignmentAction<I32> gActQuxGets343(nullptr, gElemQux, gExpr343);
static AssignmentAction<F64> gBarGets1p62(nullptr, gElemBar, gExpr1p62);
static TransitionAction gActTransState1(&gExprBazEqualsFalse, 0);
static AssignmentAction<I32> gQuxGets97(nullptr, gElemQux, gExpr97);

static IAction* gState2EntryActs[] =
{
    &gActQuxGets343,
    nullptr
};
static IAction* gState2StepActs[] =
{
    &gBarGets1p62,
    &gActTransState1,
    nullptr
};
static IAction* gState2ExitActs[] =
{
    &gQuxGets97,
    nullptr
};
static StateMachine::StateConfig gState2Config =
{
    2,
    {gState2EntryActs},
    {gState2StepActs},
    nullptr,
    {gState2ExitActs}
};

// State configs.
static StateMachine::StateConfig gStateConfigs[] =
{
    gState1Config,
    gState2Config,
    {}
};

// State machine config.
static StateMachine::Config gSmConfig = {gStateConfigs, &gElemFoo};

/*********************************** TESTS ************************************/

TEST_GROUP(StateMachineBasic)
{
    void setup()
    {
        // Set initial state 1.
        gElemFoo.write(1);
    }

    void teardown()
    {
        // Zero state vector.
        std::memset(&gSvBacking, 0, sizeof(gSvBacking));
    }
};

TEST(StateMachineBasic, EntryLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Element `qux` gets written when the state 1 entry label runs.
    CHECK_SUCCESS(sm.step(0));
    CHECK_EQUAL(1400, gElemQux.read());

    // Zero `qux` and step the state machine again. `qux` stays 0.
    gElemQux.write(0);
    CHECK_SUCCESS(sm.step(1));
    CHECK_EQUAL(0, gElemQux.read());

    // State machine is still in state 1.
    CHECK_EQUAL(1, gElemFoo.read());
}

TEST(StateMachineBasic, StepLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Elements `bar` and `baz` get written when the state 1 step label runs.
    CHECK_SUCCESS(sm.step(0));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // Zero the elements.
    gElemBar.write(0.0);
    gElemBaz.write(false);

    // Step the state machine again. The elements get written again.
    CHECK_SUCCESS(sm.step(1));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // State machine is still in state 1.
    CHECK_EQUAL(1, gElemFoo.read());
}

TEST(StateMachineBasic, RangeLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Values of `bar` and `baz` are initially those set in the step label.
    CHECK_SUCCESS(sm.step(0));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // Just before the start of the range, `bar` and `baz` are still unchanged.
    CHECK_SUCCESS(sm.step(99));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // On the first step of the range, `bar` and `baz` get overwritten.
    CHECK_SUCCESS(sm.step(100));
    CHECK_EQUAL(7.777, gElemBar.read());
    CHECK_EQUAL(false, gElemBaz.read());

    // On the last step of the range, `bar` and `baz` are still overwritten.
    CHECK_SUCCESS(sm.step(200));
    CHECK_EQUAL(7.777, gElemBar.read());
    CHECK_EQUAL(false, gElemBaz.read());

    // Beyond the range, `bar` and `baz` return to the values set in step label.
    CHECK_SUCCESS(sm.step(201));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());
}

TEST(StateMachineBasic, TransitionAndExitLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Just `qux == 200` does not trigger transition to state 2.
    gElemQux.write(200);
    CHECK_SUCCESS(sm.step(0));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(1, gElemFoo.read());

    // Just `bar < 0.0`does not trigger transition to state 2.
    gElemQux.write(0);
    gElemBar.write(-1.0);
    CHECK_SUCCESS(sm.step(1));
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(1, gElemFoo.read());

    // Trigger transition to state 2. The exit label should set `bar` to 1.522.
    // Afterwards, `qux` should be unchanged since state 2 has not started yet.
    gElemQux.write(200);
    gElemBar.write(-1.0);
    CHECK_SUCCESS(sm.step(2));
    CHECK_EQUAL(1.522, gElemBar.read());
    CHECK_EQUAL(1, gElemFoo.read());
    CHECK_EQUAL(200, gElemQux.read());

    // Step again. State 1 entry and step labels execute.
    CHECK_SUCCESS(sm.step(3));
    CHECK_EQUAL(343, gElemQux.read());
    CHECK_EQUAL(1.62, gElemBar.read());
    CHECK_EQUAL(2, gElemFoo.read());
}
