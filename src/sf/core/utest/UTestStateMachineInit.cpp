////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/utest/UTestStateMachineInit.cpp
/// @brief Unit tests for StateMachine::init().
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/StateMachine.hpp"
#include "sf/utest/UTest.hpp"

///////////////////////////// State Machine Config /////////////////////////////

/*
[local]
bool foo
bool bar
bool baz

[State1]
.entry
    foo: -> State2
.step
    foo {
        bar {
            baz {
                -> State2
            }
        }
        ELSE: -> State2
    }
    -> State2
.exit

[State2]
*/

// State vector
static struct
{
    bool foo;
    bool bar;
    bool baz;
    U32 state;
    U64 stateTime;
    U64 globalTime;
} gSvBacking;

static Element<bool> gElemFoo(gSvBacking.foo);
static Element<bool> gElemBar(gSvBacking.bar);
static Element<bool> gElemBaz(gSvBacking.baz);
static Element<U32> gElemState(gSvBacking.state);
static Element<U64> gElemStateTime(gSvBacking.stateTime);
static Element<U64> gElemGlobalTime(gSvBacking.globalTime);

static ElementExprNode<bool> gExprFoo(gElemFoo);
static ElementExprNode<bool> gExprBar(gElemBar);
static ElementExprNode<bool> gExprBaz(gElemBaz);

// -> State2
static TransitionAction gTransToState2(2);

// State 1 blocks
static StateMachine::Block gState1EntryTransBlock =
    {nullptr, nullptr, nullptr, &gTransToState2, nullptr};

static StateMachine::Block gState1EntryBlock =
    {&gExprFoo, &gState1EntryTransBlock, nullptr, nullptr, nullptr};

static StateMachine::Block gState1StepTransBlock =
    {nullptr, nullptr, nullptr, &gTransToState2, nullptr};

static StateMachine::Block gState1StepBazBlock =
    {&gExprBaz, &gState1StepTransBlock, nullptr, nullptr, nullptr};

static StateMachine::Block gState1StepBarElseBlock =
    {nullptr, nullptr, nullptr, &gTransToState2, nullptr};

static StateMachine::Block gState1StepBarBlock =
{
    &gExprBar,
    &gState1StepBazBlock,
    &gState1StepBarElseBlock,
    nullptr,
    nullptr
};

static StateMachine::Block gState1StepNextBlock =
    {nullptr, nullptr, nullptr, &gTransToState2, nullptr};

static StateMachine::Block gState1StepBlock =
    {&gExprFoo, &gState1StepBarBlock, nullptr, nullptr, &gState1StepNextBlock};

static StateMachine::Block gState1ExitBlock =
    {nullptr, nullptr, nullptr, nullptr, nullptr};

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

///
/// @brief Unit tests for StateMachine::init().
///
TEST_GROUP(StateMachineInit)
{
    void setup()
    {
        // Zero out the state vector.
        gSvBacking = {false, false, false, 0, 0, 0};
    }
};

///
/// @test State machine initialization succeeds with a valid config.
///
TEST(StateMachineInit, Init)
{
    StateMachine sm;
    gElemState.write(1);
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));
    CHECK_SUCCESS(sm.step());
}

///
/// @test Stepping an uninitialized state machine returns an error.
///
TEST(StateMachineInit, Uninitialized)
{
    StateMachine sm;
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test Initializing a state machine twice returns an error.
///
TEST(StateMachineInit, ErrorReinitialize)
{
    StateMachine sm;
    gElemState.write(1);
    CHECK_SUCCESS(StateMachine::init(gConfig, sm));
    CHECK_ERROR(E_SM_REINIT, StateMachine::init(gConfig, sm));
}

///
/// @test Initializing a state machine with an invalid initial state returns an
/// error.
///
TEST(StateMachineInit, ErrorInvalidInitialState)
{
    StateMachine sm;
    CHECK_ERROR(E_SM_STATE, StateMachine::init(gConfig, sm));
}

///
/// @test Initializing a state machine with a null state element returns an
/// error.
///
TEST(StateMachineInit, ErrorNullStateElem)
{
    StateMachine sm;
    gElemState.write(1);

    auto stash = gConfig.elemState;
    gConfig.elemState = nullptr;
    const Result res = StateMachine::init(gConfig, sm);
    gConfig.elemState = stash;

    CHECK_ERROR(E_SM_NULL, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test Initializing a state machine with a null state time element returns an
/// error.
///
TEST(StateMachineInit, ErrorNullStateTimeElem)
{
    StateMachine sm;
    gElemState.write(1);

    auto stash = gConfig.elemStateTime;
    gConfig.elemStateTime = nullptr;
    const Result res = StateMachine::init(gConfig, sm);
    gConfig.elemStateTime = stash;

    CHECK_ERROR(E_SM_NULL, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test Initializing a state machine with a null global time element returns
/// an error.
///
TEST(StateMachineInit, ErrorNullGlobalTimeElem)
{
    StateMachine sm;
    gElemState.write(1);

    auto stash = gConfig.elemGlobalTime;
    gConfig.elemGlobalTime = nullptr;
    const Result res = StateMachine::init(gConfig, sm);
    gConfig.elemGlobalTime = stash;

    CHECK_ERROR(E_SM_NULL, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test Initializing a state machine with a null state config array returns
/// an error.
///
TEST(StateMachineInit, ErrorNullStatesArray)
{
    StateMachine sm;
    gElemState.write(1);

    auto stash = gConfig.states;
    gConfig.states = nullptr;
    const Result res = StateMachine::init(gConfig, sm);
    gConfig.states = stash;

    CHECK_ERROR(E_SM_NULL, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test Initializing a state machine with an empty state config array returns
/// an error.
///
TEST(StateMachineInit, ErrorEmptyStatesArray)
{
    StateMachine sm;
    gElemState.write(1);

    StateMachine::StateConfig emptyStates[] =
    {
        {0, nullptr, nullptr, nullptr}
    };

    auto stash = gConfig.states;
    gConfig.states = emptyStates;
    const Result res = StateMachine::init(gConfig, sm);
    gConfig.states = stash;

    CHECK_ERROR(E_SM_EMPTY, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test A transition to an invalid state in an entry label returns an error
/// on initialization.
///
TEST(StateMachineInit, ErrorInvalidTransitionInEntryLabel)
{
    StateMachine sm;
    gElemState.write(1);

    TransitionAction badTrans(3);

    auto stash = gState1EntryTransBlock.action;
    gState1EntryTransBlock.action = &badTrans;
    const Result res = StateMachine::init(gConfig, sm);
    gState1EntryTransBlock.action = stash;

    CHECK_ERROR(E_SM_TRANS, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test A transition to an invalid state in a step label if branch returns an
/// error on initialization.
///
TEST(StateMachineInit, ErrorInvalidTransitionInStepLabelIfBlock)
{
    StateMachine sm;
    gElemState.write(1);

    TransitionAction badTrans(3);

    auto stash = gState1StepTransBlock.action;
    gState1StepTransBlock.action = &badTrans;
    const Result res = StateMachine::init(gConfig, sm);
    gState1StepTransBlock.action = stash;

    CHECK_ERROR(E_SM_TRANS, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test A transition to an invalid state in a step label else branch returns
/// an error on initialization.
///
TEST(StateMachineInit, ErrorInvalidTransitionInStepLabelElseBlock)
{
    StateMachine sm;
    gElemState.write(1);

    TransitionAction badTrans(3);

    auto stash = gState1StepBarElseBlock.action;
    gState1StepBarElseBlock.action = &badTrans;
    const Result res = StateMachine::init(gConfig, sm);
    gState1StepBarElseBlock.action = stash;

    CHECK_ERROR(E_SM_TRANS, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @test A transition to an invalid state in the middle of a step label (i.e.,
/// not the first block) returns an error on initialization.
///
TEST(StateMachineInit, ErrorInvalidTransitionInStepLabelNextBlock)
{
    StateMachine sm;
    gElemState.write(1);

    TransitionAction badTrans(3);

    auto stash = gState1StepNextBlock.action;
    gState1StepNextBlock.action = &badTrans;
    const Result res = StateMachine::init(gConfig, sm);
    gState1StepNextBlock.action = stash;

    CHECK_ERROR(E_SM_TRANS, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}

///
/// @brief A transition in an exit label returns an error on initialization.
///
TEST(StateMachineInit, ErrorIllegalTransitionInExitLabel)
{
    StateMachine sm;
    gElemState.write(1);

    TransitionAction trans(2);

    auto stash = gState1ExitBlock.action;
    gState1ExitBlock.action = &trans;
    const Result res = StateMachine::init(gConfig, sm);
    gState1ExitBlock.action = stash;

    CHECK_ERROR(E_SM_TR_EXIT, res);
    CHECK_ERROR(E_SM_UNINIT, sm.step());
}
