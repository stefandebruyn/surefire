#include "sfa/sm/TransitionAction.hpp"
#include "utest/UTest.hpp"

TEST_GROUP(TransitionAction)
{
};

TEST(TransitionAction, SetTransitionState)
{
    StateVector sv({nullptr, nullptr});
    TransitionAction act(sv, nullptr, 10);
    CHECK_EQUAL(10, act.transitionState);
}

TEST(TransitionAction, EvaluateNoGuard)
{
    StateVector sv({nullptr, nullptr});
    TransitionAction act(sv, nullptr, 0);
    bool trans = false;
    CHECK_SUCCESS(act.evaluate(trans));
    CHECK_EQUAL(true, trans);
}

TEST(TransitionAction, EvaluateGuard)
{
    StateVector sv({nullptr, nullptr});
    ExpressionTree<bool> guardTrue(true);
    ExpressionTree<bool> guardFalse(false);

    TransitionAction actExec(sv, &guardTrue, 0);
    bool trans = false;
    CHECK_SUCCESS(actExec.evaluate(trans));
    CHECK_EQUAL(true, trans);

    trans = true;
    TransitionAction actNoExec(sv, &guardFalse, 0);
    CHECK_SUCCESS(actNoExec.evaluate(trans));
    CHECK_EQUAL(false, trans);
}
