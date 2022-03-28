#include "sf/core/Assert.hpp"
#include "sf/core/MemOps.hpp"
#include "sf/pal/Console.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static Result foo(const bool kFail)
{
    SF_SAFE_ASSERT(kFail);

    return SUCCESS;
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(Assert)
{
    void teardown()
    {
        // Reset assert fail site.
        Assert::failFile = nullptr;
        Assert::failLineNum = -1;
    }
};

TEST(Assert, SafeAssert)
{
    // No assert fail site.
    CHECK_TRUE(Assert::failFile == nullptr);
    CHECK_EQUAL(-1, Assert::failLineNum);

    // Assert pass case.
    CHECK_SUCCESS(foo(true));

    // Assert fail case.
    CHECK_ERROR(E_ASSERT, foo(false));

#ifdef SF_SAFE_ASSERT_SAVES_FAIL_SITE
    // Assert fail site was saved since the appropriate symbol is defined.
    Console::printf(" Note: assert fail site saving is enabled");
    CHECK_TRUE(Assert::failFile != nullptr);
    CHECK_TRUE(Assert::failLineNum != -1);
#else
    // Assert fail site still not saved since the symbol is not defined.
    Console::printf(" Note: assert fail site saving is disabled");
    CHECK_TRUE(Assert::failFile == nullptr);
    CHECK_EQUAL(-1, Assert::failLineNum);
#endif
}

TEST(Assert, UnsafeAssert)
{
    // Asserting true never halts.
    SF_ASSERT(true);

#ifndef SF_ENABLE_ASSERTS
    // Asserting false does not halt when asserts are not enabled.
    Console::printf(" Note: asserts are disabled");
    SF_ASSERT(false);
#else
    Console::printf(" Note: asserts are enabled");
#endif
}
