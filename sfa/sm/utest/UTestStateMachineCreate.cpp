#include "sfa/sm/StateMachine.hpp"
#include "sfa/sm/TransitionAction.hpp"
#include "utest/UTest.hpp"

//////////////////////////// State Machine Config //////////////////////////////

static IAction* gRangeLabelActions[] = {nullptr};

static StateMachine::LabelConfig gRangeLabelConfigs[] =
{
    {gRangeLabelActions, 0, 10},
    {}
};

static StateMachine::StateConfig gStateConfigs[] =
{
    {1, {}, {}, gRangeLabelConfigs, {}},
    {2, {}, {}, gRangeLabelConfigs, {}},
    {0, nullptr}
};

static U32 gElemStateBacking = 1;
static U64 gElemGlobalTimeBacking = 0;

static Element<U32> gElemState(gElemStateBacking);
static Element<U64> gElemGlobalTime(gElemGlobalTimeBacking);

static StateMachine::Config gConfig =
{
    gStateConfigs,
    &gElemState,
    &gElemGlobalTime,
    nullptr
};

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineCreate)
{
};

TEST(StateMachineCreate, Success)
{
    StateMachine sm;
    CHECK_SUCCESS(StateMachine::create(gConfig, sm));
    CHECK_SUCCESS(sm.step());
}

TEST(StateMachineCreate, ConfigErrorNullElemState)
{
    Element<U32>* tmp = gConfig.elemState;
    gConfig.elemState = nullptr;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.elemState = tmp;

    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorNullElemGlobalTime)
{
    const Element<U64>* tmp = gConfig.elemGlobalTime;
    gConfig.elemGlobalTime = nullptr;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.elemGlobalTime = tmp;

    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorNullStateArray)
{
    StateMachine::StateConfig* tmp = gConfig.states;
    gConfig.states = nullptr;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.states = tmp;

    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorDuplicateStateId)
{
    const U32 tmp = gConfig.states[1].id;
    gConfig.states[1].id = 1;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.states[1].id = tmp;

    CHECK_EQUAL(E_DUPLICATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorNoStates)
{
    const U32 tmp = gConfig.states[0].id;
    gConfig.states[0].id = 0;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.states[0].id = tmp;

    CHECK_EQUAL(E_STATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorInvalidLabelRange)
{
    const U64 tmp = gRangeLabelConfigs[0].rangeLower;
    gRangeLabelConfigs[0].rangeLower = 11;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gRangeLabelConfigs[0].rangeLower = tmp;

    CHECK_EQUAL(E_RANGE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorTransitionInExitLabel)
{
    IAction** tmp = gStateConfigs[0].exitLabel.actions;
    TransitionAction act(nullptr, 1);
    IAction* acts[] = {&act, nullptr};
    gStateConfigs[0].exitLabel.actions = acts;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gStateConfigs[0].exitLabel.actions = tmp;

    CHECK_EQUAL(E_TRANSITION, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorInvalidTransitionInEntryLabel)
{
    IAction** tmp = gStateConfigs[0].entryLabel.actions;
    TransitionAction act(nullptr, 3);
    IAction* acts[] = {&act, nullptr};
    gStateConfigs[0].entryLabel.actions = acts;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gStateConfigs[0].entryLabel.actions = tmp;

    CHECK_EQUAL(E_STATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorInvalidTransitionInStepLabel)
{
    IAction** tmp = gStateConfigs[0].stepLabel.actions;
    TransitionAction act(nullptr, 3);
    IAction* acts[] = {&act, nullptr};
    gStateConfigs[0].stepLabel.actions = acts;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gStateConfigs[0].stepLabel.actions = tmp;

    CHECK_EQUAL(E_STATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}

TEST(StateMachineCreate, ConfigErrorInvalidTransitionInRangeLabel)
{
    IAction** tmp = gStateConfigs[0].rangeLabels[0].actions;
    TransitionAction act(nullptr, 3);
    IAction* acts[] = {&act, nullptr};
    gStateConfigs[0].rangeLabels[0].actions = acts;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gStateConfigs[0].rangeLabels[0].actions = tmp;

    CHECK_EQUAL(E_STATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step());
}
