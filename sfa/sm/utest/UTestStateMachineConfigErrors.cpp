#include "sfa/sm/StateMachine.hpp"
#include "sfa/sm/TransitionAction.hpp"
#include "utest/UTest.hpp"

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

static U32 gStateElemBacking = 1;

static Element<U32> gStateElem(gStateElemBacking);

static StateMachine::Config gConfig =
{
    gStateConfigs,
    &gStateElem
};

TEST_GROUP(StateMachineConfigErrors)
{
};

TEST(StateMachineConfigErrors, NullStateElem)
{
    Element<U32>* tmp = gConfig.eState;
    gConfig.eState = nullptr;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.eState = tmp;
    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, NullStateArray)
{
    StateMachine::StateConfig* tmp = gConfig.states;
    gConfig.states = nullptr;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.states = tmp;
    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, DuplicateStateId)
{
    const U32 tmp = gConfig.states[1].id;
    gConfig.states[1].id = 1;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.states[1].id = tmp;
    CHECK_EQUAL(E_DUPLICATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, ReservedStateId)
{
    const U32 tmp = gConfig.states[1].id;
    gConfig.states[1].id = 0xFFFFFFFF;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gConfig.states[1].id = tmp;
    CHECK_EQUAL(E_RESERVED, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, InvalidLabelRange)
{
    const U64 tmp = gRangeLabelConfigs[0].rangeLower;
    gRangeLabelConfigs[0].rangeLower = 11;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gRangeLabelConfigs[0].rangeLower = tmp;
    CHECK_EQUAL(E_RANGE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, TransitionInExitLabel)
{
    IAction** tmp = gStateConfigs[0].exitLabel.actions;
    TransitionAction act(nullptr, 1);
    IAction* acts[] = {&act, nullptr};
    gStateConfigs[0].exitLabel.actions = acts;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gStateConfigs[0].exitLabel.actions = tmp;
    CHECK_EQUAL(E_TRANSITION, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}
