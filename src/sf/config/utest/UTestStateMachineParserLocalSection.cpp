////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/utest/UTestStateMachineParserLocalSection.hpp
/// @brief Unit tests for StateMachineParser parsing local sections.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/StateMachineParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that parsing a state machine generates a certain error.
///
/// @param[in] kIt       State machine config to parse.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error column number.
/// @param[in] kColNum   Expected error line number.
///
static void checkParseError(TokenIterator& kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    Vec<StateMachineParse::LocalElementParse> parse;
    ErrorInfo err;
    TokenIterator itCpy = kIt;
    CHECK_ERROR(kRes, StateMachineParser::parseLocalSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateMachineParser::parseLocalSection(itCpy,
                                                            parse,
                                                            nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

///
/// @brief Unit tests for StateMachineParser parsing local sections.
///
TEST_GROUP(StateMachineParserLocalSection)
{
};

///
/// @test An empty local section is parsed correctly.
///
TEST(StateMachineParserLocalSection, Empty)
{
    // Parse local section.
    TOKENIZE("[local]");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(0, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

///
/// @test An empty local section containing newlines is parsed correctly.
///
TEST(StateMachineParserLocalSection, EmptyWithNewlines)
{
    TOKENIZE("[local]\n\n\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));
    CHECK_EQUAL(0, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

///
/// @test A local section containing a single element is parsed correctly.
///
TEST(StateMachineParserLocalSection, OneElement)
{
    // Parse local section.
    TOKENIZE(
        "[local]\n"
        "I32 foo = 0\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 0
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);
}

///
/// @test A read-only annotation is parsed correctly.
///
TEST(StateMachineParserLocalSection, ReadOnlyAnnotation)
{
    // Parse local section.
    TOKENIZE(
        "[local]\n"
        "I32 foo = 0 @read_only\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 0
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(parse[0].readOnly);
}

///
/// @test Multiple elements are parsed correctly.
///
TEST(StateMachineParserLocalSection, MultipleElements)
{
    // Parse local section.
    TOKENIZE(
        "[local]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0\n"
        "bool baz = false\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(3, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 10
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // F64 bar = 0.0
    CHECK_TRUE(parse[1].tokType == toks[7]);
    CHECK_TRUE(parse[1].tokName == toks[8]);
    node = parse[1].initValExpr;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // BOOL baz = FALSE
    CHECK_TRUE(parse[2].tokType == toks[12]);
    CHECK_TRUE(parse[2].tokName == toks[13]);
    node = parse[2].initValExpr;
    CHECK_EQUAL(node->data, toks[15]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);
}

///
/// @test Multiple elements with annotations are parsed correctly.
///
TEST(StateMachineParserLocalSection, MultipleElementsWithAnnotations)
{
    // Parse local section.
    TOKENIZE(
        "[local]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0 @read_only\n"
        "bool baz = false @read_only\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(3, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 10
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // F64 bar = 0.0 @read_only
    CHECK_TRUE(parse[1].tokType == toks[7]);
    CHECK_TRUE(parse[1].tokName == toks[8]);
    node = parse[1].initValExpr;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(parse[1].readOnly);

    // BOOL baz = FALSE @read_only
    CHECK_TRUE(parse[2].tokType == toks[13]);
    CHECK_TRUE(parse[2].tokName == toks[14]);
    node = parse[2].initValExpr;
    CHECK_EQUAL(node->data, toks[16]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(parse[2].readOnly);
}

///
/// @test All element types are parsed correctly.
///
TEST(StateMachineParserLocalSection, AllElementTypes)
{
    // Parse local section.
    TOKENIZE(
        "[local]\n"
        "I8 a = 0\n"
        "I16 b = 0\n"
        "I32 c = 0\n"
        "I64 d = 0\n"
        "U8 e = 0\n"
        "U16 f = 0\n"
        "U32 g = 0\n"
        "U64 h = 0\n"
        "F32 i = 0.0\n"
        "F64 j = 0.0\n"
        "bool k = false\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(11, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I8 a = 0
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // I16 b = 0
    CHECK_TRUE(parse[1].tokType == toks[7]);
    CHECK_TRUE(parse[1].tokName == toks[8]);
    node = parse[1].initValExpr;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[1].readOnly);

    // I32 c = 0
    CHECK_TRUE(parse[2].tokType == toks[12]);
    CHECK_TRUE(parse[2].tokName == toks[13]);
    node = parse[2].initValExpr;
    CHECK_EQUAL(node->data, toks[15]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[2].readOnly);

    // I64 d = 0
    CHECK_TRUE(parse[3].tokType == toks[17]);
    CHECK_TRUE(parse[3].tokName == toks[18]);
    node = parse[3].initValExpr;
    CHECK_EQUAL(node->data, toks[20]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[3].readOnly);

    // U8 e = 0
    CHECK_TRUE(parse[4].tokType == toks[22]);
    CHECK_TRUE(parse[4].tokName == toks[23]);
    node = parse[4].initValExpr;
    CHECK_EQUAL(node->data, toks[25]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[4].readOnly);

    // U16 f = 0
    CHECK_TRUE(parse[5].tokType == toks[27]);
    CHECK_TRUE(parse[5].tokName == toks[28]);
    node = parse[5].initValExpr;
    CHECK_EQUAL(node->data, toks[30]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[5].readOnly);

    // U32 g = 0
    CHECK_TRUE(parse[6].tokType == toks[32]);
    CHECK_TRUE(parse[6].tokName == toks[33]);
    node = parse[6].initValExpr;
    CHECK_EQUAL(node->data, toks[35]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[6].readOnly);

    // U64 h = 0
    CHECK_TRUE(parse[7].tokType == toks[37]);
    CHECK_TRUE(parse[7].tokName == toks[38]);
    node = parse[7].initValExpr;
    CHECK_EQUAL(node->data, toks[40]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[7].readOnly);

    // F32 i = 0
    CHECK_TRUE(parse[8].tokType == toks[42]);
    CHECK_TRUE(parse[8].tokName == toks[43]);
    node = parse[8].initValExpr;
    CHECK_EQUAL(node->data, toks[45]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[8].readOnly);

    // F64 j = 0
    CHECK_TRUE(parse[9].tokType == toks[47]);
    CHECK_TRUE(parse[9].tokName == toks[48]);
    node = parse[9].initValExpr;
    CHECK_EQUAL(node->data, toks[50]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[9].readOnly);

    // BOOL k = FALSE
    CHECK_TRUE(parse[10].tokType == toks[52]);
    CHECK_TRUE(parse[10].tokName == toks[53]);
    node = parse[10].initValExpr;
    CHECK_EQUAL(node->data, toks[55]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[10].readOnly);
}

///
/// @test A local element assignment expression with multiple terms is parsed
/// correctly.
///
TEST(StateMachineParserLocalSection, MultipleTermsInElementAssignment)
{
    // Parse local section.
    TOKENIZE(
        "[local]\n"
        "I32 foo = a + b\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = a + b
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[6]);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);
    node = parse[0].initValExpr->left;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    node = parse[0].initValExpr->right;
    CHECK_EQUAL(node->data, toks[7]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);
}

///////////////////////////////// Error Tests //////////////////////////////////

///
/// @brief Unit tests for StateMachineParser parsing local sections with errors.
///
TEST_GROUP(StateMachineParserLocalSectionErrors)
{
};

///
/// @test Multiple read-only annotations on the same element generate an error.
///
TEST(StateMachineParserLocalSectionErrors, RedundantReadOnlyAnnotation)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo = 0 @read_only @read_only\n");
    checkParseError(it, E_SMP_RO_MULT, 2, 24);
}

///
/// @test A non-identifier token where an element type is expected generates an
/// error.
///
TEST(StateMachineParserLocalSectionErrors, ExpectedElementType)
{
    TOKENIZE(
        "[local]\n"
        "@foo bar = 0\n");
    checkParseError(it, E_SMP_ELEM_TYPE, 2, 1);
}

///
/// @test No tokens after the element type generates an error.
///
TEST(StateMachineParserLocalSectionErrors, EofAfterElementType)
{
    TOKENIZE(
        "[local]\n"
        "I32\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 1);
}

///
/// @test A non-identifier token where an element identifier is expected
/// generates an error.
///
TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterElementType)
{
    TOKENIZE(
        "[local]\n"
        "I32 @foo = 0\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 1);
}

///
/// @test No tokens after the element identifier generates an error.
///
TEST(StateMachineParserLocalSectionErrors, EofAfterElementName)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo\n");
    checkParseError(it, E_SMP_LOC_OP, 2, 5);
}

///
/// @test A non-operator token where the assignment operator is expected
/// generates an error.
///
TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo @foo\n 0");
    checkParseError(it, E_SMP_LOC_OP, 2, 5);
}

///
/// @test An operator other than the assignment operator generates an error.
///
TEST(StateMachineParserLocalSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo > 0\n");
    checkParseError(it, E_SMP_LOC_OP, 2, 5);
}

///
/// @test No tokens after the assignment operator generates an error.
///
TEST(StateMachineParserLocalSectionErrors, EofAfterAssignmentOp)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo =\n");
    checkParseError(it, E_SMP_LOC_VAL, 2, 9);
}

///
/// @test An invalid expression after the assignment operator generates an
/// error.
///
TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterAssignmentOp)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo = @foo\n");
    checkParseError(it, E_SMP_LOC_VAL, 2, 9);
}

///
/// @test An unknown annotation generates an error.
///
TEST(StateMachineParserLocalSectionErrors, UnknownAnnotation)
{
    TOKENIZE(
        "[local]\n"
        "I32 foo = 0 @foo\n");
    checkParseError(it, E_SMP_ANNOT, 2, 13);
}
