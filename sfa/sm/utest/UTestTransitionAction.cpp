#include "sfa/sm/TransitionAction.hpp"
#include "utest/UTest.hpp"

TEST_GROUP(TransitionAction)
{
};

TEST(TransitionAction, SetTransitionState)
{
    TransitionAction act(nullptr, 10);
    CHECK_EQUAL(10, act.destinationState);
}

TEST(TransitionAction, EvaluateNoGuard)
{
    TransitionAction act(nullptr, 0);
    bool trans = false;
    CHECK_SUCCESS(act.evaluate(trans));
    CHECK_EQUAL(true, trans);
}

TEST(TransitionAction, EvaluateGuard)
{
    ExpressionNode<bool> guardTrue(true);
    ExpressionNode<bool> guardFalse(false);

    TransitionAction actExec(&guardTrue, 0);
    bool trans = false;
    CHECK_SUCCESS(actExec.evaluate(trans));
    CHECK_EQUAL(true, trans);

    trans = true;
    TransitionAction actNoExec(&guardFalse, 0);
    CHECK_SUCCESS(actNoExec.evaluate(trans));
    CHECK_EQUAL(false, trans);
}
