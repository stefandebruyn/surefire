#include <cstring>

#include "sfa/core/StateMachine.hpp"
#include "sfa/core/TransitionAction.hpp"
#include "sfa/core/AssignmentAction.hpp"
#include "sfa/core/StateVector.hpp"
#include "UTest.hpp"

///////////////////////////// State Vector Config //////////////////////////////

#pragma pack(push, 1)
static struct
{
    U32 state;
    U64 globalTime;
    F64 bar;
    bool baz;
    I32 qux;
} gSvBacking;
#pragma pack(pop)

static Element<U32> gElemState(gSvBacking.state);
static Element<U64> gElemGlobalTime(gSvBacking.globalTime);
static Element<F64> gElemBar(gSvBacking.bar);
static Element<bool> gElemBaz(gSvBacking.baz);
static Element<I32> gElemQux(gSvBacking.qux);

//////////////////////////// State Machine Config //////////////////////////////

/*
[STATE/State1]
ENTRY:
    qux = 1400
STEP:
    baz = true
    (qux == 200 AND bar < 0.0): -> State2
    bar = 9.81
T[100, 200]:
    bar = 7.777
    baz = false
EXIT:
    bar = 1.522
*/
static ExpressionNode<I32> gExpr1400(1400);
static ExpressionNode<F64> gExpr9p81(9.81);
static ExpressionNode<bool> gExprTrue(true);
static ExpressionNode<I32> gExpr200(200);
static ExpressionNode<F64> gExpr0p0(0.0);
static ExpressionNode<F64> gExpr7p777(7.777);
static ExpressionNode<bool> gExprFalse(false);
static ExpressionNode<F64> gExpr1p522(1.522);

static ExpressionNode<F64> gExprBar(gElemBar);
static ExpressionNode<bool> gExprBaz(gElemBaz);
static ExpressionNode<I32> gExprQux(gElemQux);

static ExpressionNode<bool, I32> gExprQuxEquals200(OP_EQUALS,
                                                   &gExprQux,
                                                   &gExpr200);
static ExpressionNode<bool, F64> gExprBarEquals0p0(OP_LESS_THAN,
                                                   &gExprBar,
                                                   &gExpr0p0);
static ExpressionNode<bool> gGuardTransState2(OP_AND,
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
    {gState1EntryActs, 0, 0},
    {gState1StepActs, 0, 0},
    gState1RangeLabels,
    {gState1ExitActs, 0, 0}
};

/*
[STATE/State2]
ENTRY:
    qux = 343
STEP:
    bar = 1.62
    (baz == false): -> State1
EXIT:
    qux = 97
*/
static ExpressionNode<I32> gExpr343(343);
static ExpressionNode<F64> gExpr1p62(1.62);
static ExpressionNode<I32> gExpr97(97);

static ExpressionNode<bool> gExprBazEqualsFalse(OP_EQUALS,
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
    {gState2EntryActs, 0, 0},
    {gState2StepActs, 0, 0},
    nullptr,
    {gState2ExitActs, 0, 0}
};

/*
This state is designed to test action and label precedence by setting `qux` to
different values in each label.

[STATE/State3]
ENTRY:
    qux = 0
    qux = 1
    baz = false
STEP:
    qux = 2
    qux = 3
    (baz == true): -> State1
T[100, 105]:
    qux = 4
    qux = 5
T[100, 100]:
    qux = 6
    qux = 7
EXIT:
    qux = 8
    qux = 9
*/
static ExpressionNode<I32> gExpr0(0);
static ExpressionNode<I32> gExpr1(1);
static ExpressionNode<I32> gExpr2(2);
static ExpressionNode<I32> gExpr3(3);
static ExpressionNode<I32> gExpr4(4);
static ExpressionNode<I32> gExpr5(5);
static ExpressionNode<I32> gExpr6(6);
static ExpressionNode<I32> gExpr7(7);
static ExpressionNode<I32> gExpr8(8);
static ExpressionNode<I32> gExpr9(9);

static AssignmentAction<I32> gActQuxGets0(nullptr, gElemQux, gExpr0);
static AssignmentAction<I32> gActQuxGets1(nullptr, gElemQux, gExpr1);
static AssignmentAction<I32> gActQuxGets2(nullptr, gElemQux, gExpr2);
static AssignmentAction<I32> gActQuxGets3(nullptr, gElemQux, gExpr3);
static AssignmentAction<I32> gActQuxGets4(nullptr, gElemQux, gExpr4);
static AssignmentAction<I32> gActQuxGets5(nullptr, gElemQux, gExpr5);
static AssignmentAction<I32> gActQuxGets6(nullptr, gElemQux, gExpr6);
static AssignmentAction<I32> gActQuxGets7(nullptr, gElemQux, gExpr7);
static AssignmentAction<I32> gActQuxGets8(nullptr, gElemQux, gExpr8);
static AssignmentAction<I32> gActQuxGets9(nullptr, gElemQux, gExpr9);

static ExpressionNode<bool> gBazIsTrue(OP_EQUALS, &gExprBaz, &gExprTrue);

static TransitionAction gActState3TransState1(&gBazIsTrue, 1);

static IAction* gState3EntryActs[] =
{
    &gActQuxGets0,
    &gActQuxGets1,
    &gActBazGetsFalse,
    nullptr
};
static IAction* gState3StepActs[] =
{
    &gActQuxGets2,
    &gActQuxGets3,
    &gActState3TransState1,
    nullptr
};
static IAction* gState3Range1Acts[] =
{
    &gActQuxGets4,
    &gActQuxGets5,
    nullptr
};
static IAction* gState3Range2Acts[] =
{
    &gActQuxGets6,
    &gActQuxGets7,
    nullptr
};
static IAction* gState3ExitActs[] =
{
    &gActQuxGets8,
    &gActQuxGets9,
    nullptr
};
static StateMachine::LabelConfig gState3RangeLabels[] =
{
    {gState3Range1Acts, 100, 105},
    {gState3Range2Acts, 100, 100},
    {}
};
static StateMachine::StateConfig gState3Config =
{
    3,
    {gState3EntryActs, 0, 0},
    {gState3StepActs, 0, 0},
    gState3RangeLabels,
    {gState3ExitActs, 0, 0}
};

// State configs.
static StateMachine::StateConfig gStateConfigs[] =
{
    gState1Config,
    gState2Config,
    gState3Config,
    {}
};

// State machine config.
static StateMachine::Config gSmConfig =
{
    gStateConfigs,
    &gElemState,
    &gElemGlobalTime,
    nullptr
};

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineStep)
{
    void setup()
    {
        // Set initial state 1.
        gElemState.write(1);
    }

    void teardown()
    {
        // Zero state vector.
        std::memset(&gSvBacking, 0, sizeof(gSvBacking));
    }
};

TEST(StateMachineStep, EntryLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Element `qux` gets written when the state 1 entry label runs.
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(1400, gElemQux.read());

    // Zero `qux` and step the state machine again. `qux` stays 0.
    gElemQux.write(0);
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(0, gElemQux.read());

    // State machine is still in state 1.
    CHECK_EQUAL(1, gElemState.read());
}

TEST(StateMachineStep, StepLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Elements `bar` and `baz` get written when the state 1 step label runs.
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // Zero the elements.
    gElemBar.write(0.0);
    gElemBaz.write(false);

    // Step the state machine again. The elements get written again.
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // State machine is still in state 1.
    CHECK_EQUAL(1, gElemState.read());
}

TEST(StateMachineStep, RangeLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Values of `bar` and `baz` are initially those set in the step label.
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // Just before the start of the range, `bar` and `baz` are still unchanged.
    gElemGlobalTime.write(99);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());

    // On the first step of the range, `bar` and `baz` get overwritten.
    gElemGlobalTime.write(100);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(7.777, gElemBar.read());
    CHECK_EQUAL(false, gElemBaz.read());

    // On the last step of the range, `bar` and `baz` are still overwritten.
    gElemGlobalTime.write(200);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(7.777, gElemBar.read());
    CHECK_EQUAL(false, gElemBaz.read());

    // Beyond the range, `bar` and `baz` return to the values set in step label.
    gElemGlobalTime.write(201);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(true, gElemBaz.read());
}

TEST(StateMachineStep, TransitionAndExitLabel)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Just `qux == 200` does not trigger transition to state 2.
    gElemQux.write(200);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(1, gElemState.read());

    // Just `bar < 0.0`does not trigger transition to state 2.
    gElemQux.write(0);
    gElemBar.write(-1.0);
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9.81, gElemBar.read());
    CHECK_EQUAL(1, gElemState.read());

    // Trigger transition to state 2. The exit label should set `bar` to 1.522.
    // Afterwards, `qux` should be unchanged since state 2 has not started yet.
    gElemQux.write(200);
    gElemBar.write(-1.0);
    gElemGlobalTime.write(2);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(1.522, gElemBar.read());
    CHECK_EQUAL(1, gElemState.read());
    CHECK_EQUAL(200, gElemQux.read());

    // Step again. State 1 entry and step labels execute.
    gElemGlobalTime.write(3);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(343, gElemQux.read());
    CHECK_EQUAL(1.62, gElemBar.read());
    CHECK_EQUAL(2, gElemState.read());
}

TEST(StateMachineStep, ActionPrecedence)
{
    // Set initial state 3.
    gElemState.write(3);

    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gSmConfig, sm));

    // Step in state 3. `qux` becomes 3 since this is the last assignment made
    // in the step label, and the range and exit labels have not run. `baz`
    // becomes false, being overwritten by the entry label. This shows that the
    // step label executes after the entry label.
    gElemBaz.write(true);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(3, gElemQux.read());
    CHECK_EQUAL(false, gElemBaz.read());

    // Step at time 100. This causes the range labels to execute for the first
    // time, and `qux` becomes 7. This shows that range labels execute after the
    // step label, and range labels execute in the order configured.
    gElemGlobalTime.write(100);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(7, gElemQux.read());

    // Step at time 101. This causes only the 2nd range label to execute, and
    // `qux` becomes 5.
    gElemGlobalTime.write(101);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(5, gElemQux.read());

    // Trigger transition to state 1. On this step, the step label, 2nd range
    // label, and exit label all execute. `qux` becomes 9 since the exit label
    // runs after all other labels.
    gElemBaz.write(true);
    gElemGlobalTime.write(102);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(9, gElemQux.read());

    // Sanity check that state machine transition from state 3 to 1.
    CHECK_EQUAL(3, gElemState.read());
    gElemGlobalTime.write(103);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(1, gElemState.read());
}
