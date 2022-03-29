///
/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.
///

#ifndef FooStateMachine_HPP
#define FooStateMachine_HPP

#include "sf/core/StateMachine.hpp"
#include "sf/core/StateVector.hpp"

namespace FooStateMachine
{

static Result init(StateMachine& kSm, StateVector& kSv)
{
    Result res = SUCCESS;

    // Local state vector
    static struct
    {
        I32 foo;
        F64 bar;
        bool baz;
        U64 T;
    } localBacking;

    static Element<I32> elemfoo(localBacking.foo);
    static Element<F64> elembar(localBacking.bar);
    static Element<bool> elembaz(localBacking.baz);
    static Element<U64> elemT(localBacking.T);

    // Referenced state vector elements
    static Element<U64>* elemtime = nullptr;
    res = kSv.getElement("time", elemtime);
    if (res != SUCCESS)
    {
        return res;
    }

    static Element<U32>* elemstate = nullptr;
    res = kSv.getElement("state", elemstate);
    if (res != SUCCESS)
    {
        return res;
    }

    // State `Foo`
    static StateMachine::Block block0{nullptr, nullptr, nullptr, nullptr, nullptr};
    static const StateMachine::StateConfig stateFooConfig{1, &block0, nullptr, nullptr};

    static const StateMachine::StateConfig stateConfigs[] =
    {
        stateFooConfig,
        {StateMachine::NO_STATE, nullptr, nullptr, nullptr}
    }

    static const StateMachine::Config smConfig{foo, &elemT, baz, stateConfigs, nullptr};

    return StateMachine::create(smConfig, kSm);
}

} // namespace FooStateMachine

#endif
