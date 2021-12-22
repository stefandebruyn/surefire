#include "sfa/sm/StateMachine.hpp"
#include "sfa/sm/TransitionAction.hpp"
#include "utest/UTest.hpp"

static IAction* const gRangeLabelActions[] = {nullptr};

static const StateMachine::LabelConfig gRangeLabelConfigs[] =
{
    {gRangeLabelActions, 0, 10},
    {}
};

static const StateMachine::StateConfig gStateConfigs[] =
{
    {1, {}, {}, gRangeLabelConfigs, {}},
    {2, {}, {}, gRangeLabelConfigs, {}},
    {}
};

static U32 gStateElemBacking = 1;

static Element<U32> gStateElem(gStateElemBacking);

static const StateMachine::Config gConfig =
{
    gStateConfigs,
    &gStateElem
};

TEST_GROUP(StateMachineConfigErrors)
{
};

TEST(StateMachineConfigErrors, NullStateElem)
{
    const StateMachine::Config config = {gStateConfigs, nullptr};
    StateMachine sm;
    const Result res = StateMachine::create(config, sm);
    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, NullStateArray)
{
    const StateMachine::Config config = {nullptr, &gStateElem};
    StateMachine sm;
    const Result res = StateMachine::create(config, sm);
    CHECK_EQUAL(E_NULLPTR, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, DuplicateStateId)
{
    const StateMachine::StateConfig stateConfigs[] =
    {
        gStateConfigs[0],
        {1},
        {}
    };
    const StateMachine::Config config = {stateConfigs, &gStateElem};
    StateMachine sm;
    const Result res = StateMachine::create(config, sm);
    CHECK_EQUAL(E_DUPLICATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, NoStates)
{
    const StateMachine::StateConfig stateConfigs[] = {{}};
    const StateMachine::Config config = {stateConfigs, &gStateElem};
    StateMachine sm;
    const Result res = StateMachine::create(config, sm);
    CHECK_EQUAL(E_STATE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, InvalidLabelRange)
{
    const StateMachine::LabelConfig rangeLabelConfigs[] =
    {
        {gRangeLabelActions, 11, 10},
        {}
    };
    const StateMachine::StateConfig stateConfigs[] =
    {
        gStateConfigs[0],
        {2, {}, {}, rangeLabelConfigs, {}},
        {}
    };
    const StateMachine::Config config = {stateConfigs, &gStateElem};
    StateMachine sm;
    const Result res = StateMachine::create(config, sm);
    CHECK_EQUAL(E_RANGE, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}

TEST(StateMachineConfigErrors, TransitionInExitLabel)
{
    TransitionAction act(nullptr, 1);
    IAction* const actions[] = {&act, nullptr};
    const StateMachine::StateConfig stateConfigs[] =
    {
        gStateConfigs[0],
        {2, {}, {}, nullptr, {actions}},
        {}
    };
    const StateMachine::Config config = {stateConfigs, &gStateElem};
    StateMachine sm;
    const Result res = StateMachine::create(config, sm);
    CHECK_EQUAL(E_TRANSITION, res);
    CHECK_EQUAL(E_UNINITIALIZED, sm.step(0));
}
