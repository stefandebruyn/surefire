#include "pal/Thread.hpp"
#include "UTest.hpp"

TEST_GROUP(Thread)
{
};

TEST(Thread, ErrorNullFunction)
{
    I32 thread = -1;
    CHECK_ERROR(E_THR_NULL,
                Thread::create(nullptr,
                               nullptr,
                               0,
                               Thread::FAIR,
                               Thread::NO_AFFINITY,
                               thread));
    CHECK_EQUAL(-1, thread);
}
