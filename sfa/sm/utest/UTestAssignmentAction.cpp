#include "sfa/sm/AssignmentAction.hpp"
#include "utest/UTest.hpp"

TEST_GROUP(AssignmentAction)
{
};

TEST(AssignmentAction, EvaluateNoGuard)
{
    StateVector sv({nullptr, nullptr});
    I32 backing = 0;
    Element<I32> elem(backing);
    const ExpressionTree<I32> expr(10);
    AssignmentAction<I32> act(sv, nullptr, elem, expr);
    bool trans = true;
    CHECK_SUCCESS(act.evaluate(trans));
    CHECK_EQUAL(10, elem.read());
    CHECK_EQUAL(false, trans);
}

TEST(AssignmentAction, EvaluateGuard)
{
    StateVector sv({nullptr, nullptr});
    ExpressionTree<bool> guardTrue(true);
    ExpressionTree<bool> guardFalse(false);
    I32 backing = 0;
    Element<I32> elem(backing);
    const ExpressionTree<I32> expr(10);
    bool trans = true;

    AssignmentAction<I32> actExec(sv, &guardTrue, elem, expr);
    CHECK_SUCCESS(actExec.evaluate(trans));
    CHECK_EQUAL(10, elem.read());
    CHECK_EQUAL(false, trans);

    elem.write(0);
    trans = true;
    AssignmentAction<I32> actNoExec(sv, &guardFalse, elem, expr);
    CHECK_SUCCESS(actNoExec.evaluate(trans));
    CHECK_EQUAL(0, elem.read());
    CHECK_EQUAL(false, trans);
}
