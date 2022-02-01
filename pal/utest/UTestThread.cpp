#include "pal/Thread.hpp"
#include "UTest.hpp"

static Result foo(void* kArgs)
{
    U64* x = (U64*) kArgs;
    *x += 1;
    return SUCCESS;
}

TEST_GROUP(Thread)
{
};

TEST(Thread, Doop)
{
    U64 x = 10;
    I32 thread = -1;
    CHECK_SUCCESS(Thread::create(foo,
                                 &x,
                                 8,
                                 Thread::DEFAULT_SCHED,
                                 Thread::NO_AFFINITY,
                                 thread));
    Result threadRes = -1;
    CHECK_SUCCESS(Thread::await(thread, &threadRes));
    CHECK_SUCCESS(threadRes);
    CHECK_EQUAL(11, x);
}
