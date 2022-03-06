#include "sfa/sup/StateVectorCompiler.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkCompileError(const std::vector<Token>& kToks,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Parse state vector config.
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(kToks, parse, nullptr));

    // Got expected return code from compiler.
    std::shared_ptr<StateVectorCompiler::Assembly> assembly;
    ConfigErrorInfo err;
    CHECK_ERROR(kRes, StateVectorCompiler::compile(parse, assembly, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Assembly pointer was not populated.
    CHECK_TRUE(assembly == nullptr);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateVectorCompilerErrors)
{
};

TEST(StateVectorCompilerErrors, DuplicateElementNameSameRegion)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 foo\n");
    checkCompileError(toks, E_SVC_ELEM_DUPE, 3, 5);
}

TEST(StateVectorCompilerErrors, DuplicateElementNameDifferentRegion)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "[Bar]\n"
        "F64 foo\n");
    checkCompileError(toks, E_SVC_ELEM_DUPE, 4, 5);
}

TEST(StateVectorCompilerErrors, DuplicateRegionName)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "[Foo]\n"
        "F64 bar\n");
    checkCompileError(toks, E_SVC_RGN_DUPE, 3, 1);
}

TEST(StateVectorCompilerErrors, EmptyRegion)
{
    TOKENIZE(
        "[Foo]\n");
    checkCompileError(toks, E_SVC_RGN_EMPTY, 1, 1);
}

TEST(StateVectorCompilerErrors, UnknownElementType)
{
    TOKENIZE(
        "[Foo]\n"
        "I33 foo\n");
    checkCompileError(toks, E_SVC_ELEM_TYPE, 2, 1);
}
