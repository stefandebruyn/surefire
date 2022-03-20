// #include "sf/config/StateScriptParser.hpp"
// #include "sf/utest/UTest.hpp"

// /////////////////////////////////// Helpers ////////////////////////////////////

// static void checkParseError(const Vec<Token>& kToks,
//                             const Result kRes,
//                             const I32 kLineNum,
//                             const I32 kColNum)
// {
//     // Got expected return code from parser.
//     StateScriptParser::Parse parse = {};
//     ErrorInfo err;
//     CHECK_ERROR(kRes, StateScriptParser::parse(kToks, parse, &err));

//     // Correct line and column numbers of error are identified.
//     CHECK_EQUAL(kLineNum, err.lineNum);
//     CHECK_EQUAL(kColNum, err.colNum);

//     // An error message was given.
//     CHECK_TRUE(err.text.size() > 0);
//     CHECK_TRUE(err.subtext.size() > 0);

//     // Parse was not populated.
//     CHECK_EQUAL(0, parse.sections.size());
// }

// //////////////////////////////////// Tests /////////////////////////////////////

// TEST_GROUP(StateScriptParser)
// {
// };

// TEST(StateScriptParser, Empty)
// {
//     TOKENIZE("\n\n\n");
//     StateScriptParser::Parse parse = {};
//     CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
//     CHECK_EQUAL(0, parse.sections.size());
// }

// TEST(StateScriptParser, EmptySection)
// {
//     TOKENIZE("[Foo]\n");
//     StateScriptParser::Parse parse = {};
//     CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
//     CHECK_EQUAL(1, parse.sections.size());
//     CHECK_EQUAL(parse.sections[0].tokName, toks[0]);
//     CHECK_TRUE(parse.sections[0].block == nullptr);
// }

// TEST(StateScriptParser, OneSection)
// {
//     TOKENIZE(
//         "[Foo]\n"
//         "foo = 1\n"
//         "bar = 2\n");
//     StateScriptParser::Parse parse = {};
//     CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
//     CHECK_EQUAL(1, parse.sections.size());

//     // `Foo` section
//     CHECK_EQUAL(parse.sections[0].tokName, toks[0]);
//     CHECK_TRUE(parse.sections[0].block != nullptr);

//     // `foo = 1` block
//     Ref<const StateMachineParser::BlockParse> block = parse.sections[0].block;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next != nullptr);

//     // `foo = 1` action
//     CHECK_EQUAL(block->action->tokRhs, toks[2]);
//     Ref<const ExpressionParser::Parse> node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[4]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);

//     // `bar = 2` block
//     block = block->next;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next == nullptr);

//     // `bar = 2` action
//     CHECK_EQUAL(block->action->tokRhs, toks[6]);
//     node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[8]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);
// }

// TEST(StateScriptParser, TwoSections)
// {
//     TOKENIZE(
//         "[Foo]\n"
//         "foo = 1\n"
//         "bar = 2\n"
//         "\n"
//         "[Bar]\n"
//         "baz = 3\n"
//         "qux = 4\n");
//     StateScriptParser::Parse parse = {};
//     CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
//     CHECK_EQUAL(2, parse.sections.size());

//     // `Foo` section
//     CHECK_EQUAL(parse.sections[0].tokName, toks[0]);
//     CHECK_TRUE(parse.sections[0].block != nullptr);

//     // `foo = 1` block
//     Ref<const StateMachineParser::BlockParse> block = parse.sections[0].block;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next != nullptr);

//     // `foo = 1` action
//     CHECK_EQUAL(block->action->tokRhs, toks[2]);
//     Ref<const ExpressionParser::Parse> node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[4]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);

//     // `bar = 2` block
//     block = block->next;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next == nullptr);

//     // `bar = 2` action
//     CHECK_EQUAL(block->action->tokRhs, toks[6]);
//     node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[8]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);

//     // `Bar` section
//     CHECK_EQUAL(parse.sections[1].tokName, toks[11]);
//     CHECK_TRUE(parse.sections[1].block != nullptr);

//     // `baz = 3` block
//     block = parse.sections[1].block;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next != nullptr);

//     // `baz = 3` action
//     CHECK_EQUAL(block->action->tokRhs, toks[13]);
//     node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[15]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);

//     // `qux = 4` block
//     block = block->next;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next == nullptr);

//     // `qux = 4` action
//     CHECK_EQUAL(block->action->tokRhs, toks[17]);
//     node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[19]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);
// }

// TEST(StateScriptParser, Assertion)
// {
//     TOKENIZE(
//         "[Foo]\n"
//         "@ASSERT foo == 1\n"
//         "bar = 2\n");
//     StateScriptParser::Parse parse = {};
//     CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
//     CHECK_EQUAL(1, parse.sections.size());

//     // `Foo` section
//     CHECK_EQUAL(parse.sections[0].tokName, toks[0]);
//     CHECK_TRUE(parse.sections[0].block != nullptr);

//     // `@ASSERT foo = 1` block
//     Ref<const StateMachineParser::BlockParse> block = parse.sections[0].block;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action == nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next != nullptr);
//     CHECK_TRUE(block->assertion != nullptr);

//     // `foo == 1` assertion
//     Ref<const ExpressionParser::Parse> node = block->assertion;
//     CHECK_EQUAL(node->data, toks[4]);
//     CHECK_TRUE(node->left != nullptr);
//     CHECK_TRUE(node->right != nullptr);
//     node = block->assertion->left;
//     CHECK_EQUAL(node->data, toks[3]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);
//     node = block->assertion->right;
//     CHECK_EQUAL(node->data, toks[5]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);

//     // `bar = 2` block
//     block = block->next;
//     CHECK_TRUE(block->guard == nullptr);
//     CHECK_TRUE(block->action != nullptr);
//     CHECK_TRUE(block->ifBlock == nullptr);
//     CHECK_TRUE(block->elseBlock == nullptr);
//     CHECK_TRUE(block->next == nullptr);
//     CHECK_TRUE(block->assertion == nullptr);

//     // `bar = 2` action
//     CHECK_EQUAL(block->action->tokRhs, toks[7]);
//     node = block->action->lhs;
//     CHECK_EQUAL(node->data, toks[9]);
//     CHECK_TRUE(node->left == nullptr);
//     CHECK_TRUE(node->right == nullptr);
// }

// TEST(StateScriptParser, ErrorExpectedSection)
// {
//     TOKENIZE("foo = 1\n");
//     checkParseError(toks, E_SSP_SEC, 1, 1);
// }

// TEST(StateScriptParser, ErrorInBlock)
// {
//     TOKENIZE(
//         "[Foo]\n"
//         "foo = 1 +\n");
//     checkParseError(toks, E_EXP_SYNTAX, 2, 9);
// }

// TEST(StateScriptParser, ErrorInAssertion)
// {
//     TOKENIZE(
//         "[Foo]\n"
//         "@ASSERT foo +\n");
//     checkParseError(toks, E_EXP_SYNTAX, 2, 13);
// }
