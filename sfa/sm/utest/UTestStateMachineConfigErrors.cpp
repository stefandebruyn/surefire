#include "sfa/sm/StateMachine.hpp"
#include "utest/UTest.hpp"

static StateMachine::LabelConfig gLabelConfigs[] =
{
    {StateMachine::LAB_RANGE, nullptr, 0, 10},
    {StateMachine::LAB_NULL, nullptr, 0, 0}
};

static StateMachine::StateConfig gStateConfigs[] =
{
    {0, gLabelConfigs},
    {1, gLabelConfigs},
    {0, nullptr}
};

static U32 gStateElemBacking;

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
    gConfig.states[1].id = 0;
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

TEST(StateMachineConfigErrors, InvalidLabelType)
{
    const StateMachine::LabelType tmp = gLabelConfigs[0].type;
    gLabelConfigs[0].type = StateMachine::LAB_LAST;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gLabelConfigs[0].type = tmp;
    CHECK_EQUAL(E_ENUM, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, InvalidLabelRange)
{
    const U64 tmp = gLabelConfigs[0].rangeLower;
    gLabelConfigs[0].rangeLower = 100;
    StateMachine sm;
    const Result res = StateMachine::create(gConfig, sm);
    gLabelConfigs[0].rangeLower = tmp;
    CHECK_EQUAL(E_RANGE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}
