#include "sf/core/StateMachine.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/utest/UTest.hpp"

///////////////////////////// State Machine Config /////////////////////////////

/*
[local]
I32 foo = 0
I32 bar = 0
I32 baz = 0

[State1]
.entry
    foo = 100
.step
    foo = foo + 1
    foo == 110: -> State1
.exit
    foo = 0

[State2]
*/

// State vector
static struct
{
    I32 foo;
    I32 bar;
    I32 baz;
    U32 state;
    U64 stateTime;
    U64 globalTime;
} gSvBacking;

static Element<I32> gElemFoo(gSvBacking.foo);
static Element<I32> gElemBar(gSvBacking.bar);
static Element<I32> gElemBaz(gSvBacking.baz);
static Element<U32> gElemState(gSvBacking.state);
static Element<U64> gElemStateTime(gSvBacking.stateTime);
static Element<U64> gElemGlobalTime(gSvBacking.globalTime);

static ElementExprNode<I32> gExprFoo(gElemFoo);
static ElementExprNode<I32> gExprBar(gElemBar);
static ElementExprNode<I32> gExprBaz(gElemBaz);

// Expression constants
static ConstExprNode<I32> g100(100);
static ConstExprNode<I32> g1(1);
static ConstExprNode<I32> g110(110);
static ConstExprNode<I32> g0(0);

// foo = 100
static AssignmentAction<I32> gFooGets100(gElemFoo, g100);

// foo = foo + 1
static BinOpExprNode<I32> gFooPlus1(
    [] (I32 a, I32 b) -> I32 { return (a + b); },
    gExprFoo,
    g1);
static AssignmentAction<I32> gFooGetsFooPlus1(gElemFoo, gFooPlus1);

// foo == 110
static BinOpExprNode<bool, I32> gFooIs200(
    [] (I32 a, I32 b) -> bool { return (a == b); },
    gExprFoo,
    g110);

// -> State1
static TransitionAction gTransToState2(2);

// foo = 0
static AssignmentAction<I32> gFooGets0(gElemFoo, g0);

// State 1 blocks
static StateMachine::Block gState1EntryBlock =
    {nullptr, nullptr, nullptr, &gFooGets100, nullptr};

static StateMachine::Block gState1TransBlock =
    {nullptr, nullptr, nullptr, &gTransToState2, nullptr};

static StateMachine::Block gState1StepBlockNext =
    {&gFooIs200, &gState1TransBlock, nullptr, nullptr, nullptr};

static StateMachine::Block gState1StepBlock =
    {nullptr, nullptr, nullptr, &gFooGetsFooPlus1, &gState1StepBlockNext};

static StateMachine::Block gState1ExitBlock =
    {nullptr, nullptr, nullptr, &gFooGets0, nullptr};

// State machine config
static StateMachine::StateConfig gStates[] =
{
    {1, &gState1EntryBlock, &gState1StepBlock, &gState1ExitBlock},
    {2, nullptr, nullptr, nullptr},
    {0, nullptr, nullptr, nullptr}
};

static StateMachine::Config gConfig =
    {&gElemState, &gElemStateTime, &gElemGlobalTime, gStates, nullptr};

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineStep)
{
    void setup()
    {
        // Zero out the state vector.
        gSvBacking = {0, 0, 0, 0, 0, 0};
    }
};

TEST(StateMachineStep, EntryAndStep)
{
    // Initialize the state machine in state 1.
    gElemState.write(1);
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));

    // Step the state machine.
    CHECK_SUCCESS(sm.step());

    // `foo` is 101 since both the entry and step labels executed once.
    CHECK_EQUAL(101, gElemFoo.read());
    CHECK_EQUAL(1, gElemState.read());
    CHECK_EQUAL(0, gElemStateTime.read());
    CHECK_EQUAL(0, gElemGlobalTime.read());

    // Step again.
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());

    // Only the step label ran, so `foo` was incremented to 102.
    CHECK_EQUAL(102,  gElemFoo.read());
    CHECK_EQUAL(1, gElemState.read());
    CHECK_EQUAL(1, gElemStateTime.read());
    CHECK_EQUAL(1, gElemGlobalTime.read());
}

TEST(StateMachineStep, TransitionAndExit)
{
    // Initialize the state machine in state 1.
    gElemState.write(1);
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));

    // Step the state machine to execute the entry label.
    CHECK_SUCCESS(sm.step());

    // Set `foo` to 109 so that the next step increments it to 110 and then the
    // transition triggers.
    gElemFoo.write(109);
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());

    // Exit label ran and reset `foo` to 0. State machine is still in state 1;
    // the next step will be the first in state 2.
    CHECK_EQUAL(0, gElemFoo.read());
    CHECK_EQUAL(1, gElemState.read());
    CHECK_EQUAL(1, gElemStateTime.read());
    CHECK_EQUAL(1, gElemGlobalTime.read());

    // Step again. State machine is now in state 2, and the state time has been
    // reset to 0.
    gElemGlobalTime.write(2);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(0, gElemFoo.read());
    CHECK_EQUAL(2, gElemState.read());
    CHECK_EQUAL(0, gElemStateTime.read());
    CHECK_EQUAL(2, gElemGlobalTime.read());
}

TEST(StateMachineStep, EmptyState)
{
    // Initialize the state machine in state 2.
    gElemState.write(2);
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));

    // Step 100 times.
    U64 t = 0;
    for (; t < 100; ++t)
    {
        gElemGlobalTime.write(t);
        CHECK_SUCCESS(sm.step());
    }

    // State vector (basically just `foo`) is untouched since state 2 is a nop.
    CHECK_EQUAL(0, gElemFoo.read());
    CHECK_EQUAL(2, gElemState.read());
    CHECK_EQUAL(99, gElemStateTime.read());
    CHECK_EQUAL(99, gElemGlobalTime.read());
}

TEST(StateMachineStep, UpdateStateTime)
{
    // Initialize the state machine in state 2.
    gElemState.write(2);
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));

    // First step is on timestep 100. State time is 0.
    gElemGlobalTime.write(100);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(0, gElemStateTime.read());

    // Next step is on timestep 120. State time is 20.
    gElemGlobalTime.write(120);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(20, gElemStateTime.read());

    // Next step is on timestep 125. State time is 25.
    gElemGlobalTime.write(125);
    CHECK_SUCCESS(sm.step());
    CHECK_EQUAL(25, gElemStateTime.read());
}

TEST(StateMachineStep, ErrorInvalidTime)
{
    // Initialize the state machine in state 1.
    gElemState.write(1);
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));

    // Stepping with an invalid global time fails.
    gElemGlobalTime.write(Clock::NO_TIME);
    CHECK_ERROR(E_SM_TIME, sm.step());

    // State vector is untouched.
    CHECK_EQUAL(0, gElemFoo.read());
    CHECK_EQUAL(1, gElemState.read());
    CHECK_EQUAL(0, gElemStateTime.read());
    CHECK_EQUAL(Clock::NO_TIME, gElemGlobalTime.read());
}

TEST(StateMachineStep, ErrorNonMonotonicTime)
{
    // Initialize the state machine.
    gElemState.write(1);
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));

    // Step once at timestep 1.
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());

    // Stepping before updating the global time fails.
    CHECK_ERROR(E_SM_TIME, sm.step());

    // Stepping with time going backwards fails.
    gElemGlobalTime.write(0);
    CHECK_ERROR(E_SM_TIME, sm.step());

    // State vector is untouched since first successful step.
    CHECK_EQUAL(101, gElemFoo.read());
    CHECK_EQUAL(1, gElemState.read());
    CHECK_EQUAL(0, gElemStateTime.read());
    CHECK_EQUAL(0, gElemGlobalTime.read());
}

TEST(StateMachineStep, UpdateExpressionStats)
{
    // State machine will update stats for elements `bar` and `baz`.
    I32 barArrA[1];
    I32 barArrB[1];
    I32 bazArrA[1];
    I32 bazArrB[1];
    ExpressionStats<I32> statsBar(gExprBar, barArrA, barArrB, 1);
    ExpressionStats<I32> statsBaz(gExprBaz, bazArrA, bazArrB, 1);
    IExpressionStats* stats[] = {&statsBar, &statsBaz, nullptr};

    // Initialize the state machine.
    gElemState.write(1);
    StateMachine sm;
    StateMachine::Config config = gConfig;
    config.stats = stats;
    CHECK_SUCCESS(StateMachine::init(config, sm));

    // Step state machine.
    gElemBar.write(1);
    gElemBaz.write(-5);
    CHECK_SUCCESS(sm.step());

    // Expression stats were updated.
    CHECK_EQUAL(1.0, statsBar.mean());
    CHECK_EQUAL(-5.0, statsBaz.mean());

    // Change `foo` and `bar` and step again.
    gElemBar.write(-10);
    gElemBaz.write(3);
    gElemGlobalTime.write(1);
    CHECK_SUCCESS(sm.step());

    // Expression stats were updated.
    CHECK_EQUAL(-10.0, statsBar.mean());
    CHECK_EQUAL(3.0, statsBaz.mean());
}
