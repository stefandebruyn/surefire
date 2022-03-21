#include <stack>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionParse.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "expression error";

/////////////////////////////////// Public /////////////////////////////////////

Result ExpressionParse::parse(TokenIterator kIt,
                              Ref<const ExpressionParse>& kParse,
                              ErrorInfo* const kErr)
{
    // Assert that the iterator is at index zero.
    SF_ASSERT(kIt.idx() == 0);

    // Check that token sequence is not empty.
    if (kIt.size() == 0)
    {
        return E_EXP_EMPTY;
    }

    // Check that parentheses are balanced.
    I32 lvl = 0;
    const Token* tokLastLvl0Paren = nullptr;
    while (!kIt.eof())
    {
        const Token& tok = kIt.take();
        if (tok.type == Token::LPAREN)
        {
            if (lvl == 0)
            {
                // Save parenthese token in case we need it for an error
                // message.
                tokLastLvl0Paren = &tok;
            }
            ++lvl;
        }
        else if (tok.type == Token::RPAREN)
        {
            --lvl;

            if (lvl < 0)
            {
                // Unbalanced parentheses.
                ConfigUtil::setError(kErr,
                                     tok,
                                     gErrText,
                                     "unbalanced parenthese");
                return E_EXP_PAREN;
            }
        }
    }

    // Check that parentheses are balanced.
    if (lvl != 0)
    {
        SF_ASSERT(tokLastLvl0Paren != nullptr);
        ConfigUtil::setError(kErr,
                             *tokLastLvl0Paren,
                             gErrText,
                             "unbalanced parenthese");
        return E_EXP_PAREN;
    }

    // Check that expression contains only identifier, constant, operator,
    // parenthese, and comma tokens.
    kIt.seek(0);
    while (!kIt.eof())
    {
        const Token& tok = kIt.take();
        if ((tok.type != Token::IDENTIFIER)
            && (tok.type != Token::CONSTANT)
            && (tok.type != Token::OPERATOR)
            && (tok.type != Token::LPAREN)
            && (tok.type != Token::RPAREN)
            && (tok.type != Token::COMMA))
        {
            // Unexpected token in expression.
            ConfigUtil::setError(kErr,
                                 tok,
                                 gErrText,
                                 "unexpected token in expression");
            return E_EXP_TOK;
        }
    }

    // At this point we know that the expression has balanced parentheses,
    // contains only known operators, and contains no unexpected token types.
    // So, the only errors to check for herein are syntax errors.
    Ref<ExpressionParse::MutNode> root;
    const Result res = ExpressionParse::parseImpl(kIt, root, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Convert tree to one of user-facing public node type.
    ExpressionParse::convertTree(root, kParse);

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

Result ExpressionParse::popSubexpression(
    std::stack<Token>& kStack,
    std::stack<Ref<ExpressionParse::MutNode>>& kNodes,
    ErrorInfo* const kErr)
{
    // Pop operator off stack.
    const Token& op = kStack.top();
    kStack.pop();
    if (op.type != Token::OPERATOR)
    {
        // Expected an operator.
        ConfigUtil::setError(kErr, op, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // Get operator info.
    SF_ASSERT(op.opInfo != nullptr);
    const OpInfo& opInfo = *op.opInfo;

    // Pop RHS from stack.
    if (kNodes.empty())
    {
        // Expected an RHS.
        ConfigUtil::setError(kErr, op, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }
    const Ref<ExpressionParse::MutNode> right = kNodes.top();
    kNodes.pop();

    // Check that RHS comes before the operator in the expression.
    if ((right->data.lineNum < op.lineNum) || (right->data.colNum < op.colNum))
    {
        // "RHS" is actually to the left of the operator. This
        // usually indicates a syntax error with a unary operator.
        ConfigUtil::setError(kErr, op, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    Ref<ExpressionParse::MutNode> left;
    if (!opInfo.unary)
    {
        // Pop LHS from stack.
        if (kNodes.empty())
        {
            // Expected an LHS.
            ConfigUtil::setError(kErr, op, gErrText, "invalid syntax");
            return E_EXP_SYNTAX;
        }
        left = kNodes.top();
        kNodes.pop();
    }

    // Push operation onto expression.
    kNodes.push(Ref<ExpressionParse::MutNode>(
        new ExpressionParse::MutNode{op, left, right, false}));

    return SUCCESS;
}

Result ExpressionParse::parseFunctionCall(TokenIterator kIt,
                                          Ref<ExpressionParse::MutNode>& kNode,
                                          ErrorInfo* const kErr)
{
    // Assert that token sequence is an identifier followed by an open
    // parenthese and ending with a close parenthese.
    SF_ASSERT(kIt.size() >= 3);
    SF_ASSERT(kIt[0].type == Token::IDENTIFIER);
    SF_ASSERT(kIt[1].type == Token::LPAREN);
    SF_ASSERT(kIt[kIt.size() - 1].type == Token::RPAREN);

    // Stores iterators for each argument expression in the function call.
    Vec<TokenIterator> argExprs;

    // Parenthese level.
    U32 lvl = 0;

    // Start parsing at index 2, the first token after the open parenthese.
    U32 idxArgStart = 2;
    kIt.seek(idxArgStart);

    while (!kIt.eof())
    {
        if (kIt.type() == Token::LPAREN)
        {
            ++lvl;
        }
        else if ((kIt.idx() != (kIt.size() - 1))
                 && (kIt.type() == Token::RPAREN))
        {
            --lvl;
        }

        // If the parenthese level is 0 and the current token is a comma or the
        // function call closing parenthese, we found the end of an argument
        // expression.
        if ((lvl == 0)
            && ((kIt.type() == Token::COMMA) || (kIt.idx() == kIt.size() - 1)))
        {
            // If the function call has arguments and the argument expression
            // contains 0 tokens, that's a syntax error (e.g., `foo(,)`);
            const bool emptyArg = ((kIt.idx() - idxArgStart) == 0);
            if (((idxArgStart != 2) || (kIt.idx() != (kIt.size() - 1)))
                && emptyArg)
            {
                ConfigUtil::setError(kErr, kIt.tok(), gErrText,
                                     "invalid syntax");
                return E_EXP_SYNTAX;
            }

            if (!emptyArg)
            {
                // Slice iterator for parsing the argument expression later.
                argExprs.push_back(kIt.slice(idxArgStart, kIt.idx()));
                // Bump starting index of next argument expression to after the
                // comma.
                idxArgStart = (kIt.idx() + 1);
            }
        }

        kIt.take();
    }

    // First token in the function call tree contains the function name.
    kNode.reset(new ExpressionParse::MutNode{kIt[0], nullptr, nullptr, true});

    // Parse argument expressions and chain them down the left subtree of
    // the function call node. The left child of each argument node is the next
    // argument, and the right child is the argument expression.
    Ref<ExpressionParse::MutNode> node = kNode;
    for (TokenIterator& argIt : argExprs)
    {
        node->left.reset(
            new ExpressionParse::MutNode{{}, nullptr, nullptr, false});
        const Result res =
            ExpressionParse::parseImpl(argIt, node->left->right, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
        node = node->left;
    }

    return SUCCESS;
}

void ExpressionParse::expandDoubleIneq(const Ref<ExpressionParse::MutNode> kNode)
{
    // Recursion base case.
    if (kNode == nullptr)
    {
        return;
    }

    if ((kNode->left != nullptr) && (kNode->right != nullptr))
    {
        if (OpInfo::relOps.find(kNode->data.str)
            != OpInfo::relOps.end())
        {
            // If this node and the left node contain a relational operator,
            // this is a double inequality. It's impossible for the right node
            // to contain a relational operator since all operators used in
            // double inequalities have the same precedence and are
            // right-associative.
            if (OpInfo::relOps.find(kNode->left->data.str)
                != OpInfo::relOps.end())
            {
                kNode->right.reset(new ExpressionParse::MutNode{
                    kNode->data, kNode->left->right, kNode->right, false});
                kNode->data =
                {
                    Token::OPERATOR,
                    OpInfo::land.str,
                    -1,
                    -1,
                    &OpInfo::land,
                    nullptr
                };
            }
        }
    }

    // Recurse into left and right subtrees.
    ExpressionParse::expandDoubleIneq(kNode->left);
    ExpressionParse::expandDoubleIneq(kNode->right);
}

Result ExpressionParse::parseImpl(TokenIterator& kIt,
                                  Ref<ExpressionParse::MutNode>& kNode,
                                  ErrorInfo* const kErr)
{
    // Copy token sequence, minus newlines, into a vector of tokens enclosed in
    // parentheses. Adding this extra pair of parentheses simplifies the
    // algorithm.
    Vec<Token> toks = {{Token::LPAREN, "(", -1, -1, nullptr, nullptr}};
    kIt.seek(0);
    while (!kIt.eof())
    {
        toks.push_back(kIt.take());
    }
    toks.push_back({Token::RPAREN, ")", -1, -1, nullptr, nullptr});

    // Stack of expression nodes yet to be installed in the binary tree.
    std::stack<Ref<ExpressionParse::MutNode>> nodes;

    // Operator and operand stack.
    std::stack<Token> stack;

    U32 i = 0;
    while (i < toks.size())
    {
        const Token& tok = toks[i];

        if (tok.type == Token::LPAREN)
        {
            // Push left parenthese onto stack.
            stack.push(tok);
        }
        else if ((tok.type == Token::IDENTIFIER)
                 || (tok.type == Token::CONSTANT))
        {
            // Push operand onto stack.
            if ((tok.type == Token::IDENTIFIER)
                && (i < (toks.size() - 1))
                && (toks[i + 1].type == Token::LPAREN))
            {
                // Token is a function call.

                // Find index of function closing parenthese.
                U32 lvl = 0;
                U32 j = i;
                for (; j < toks.size(); ++j)
                {
                    if (toks[j].type == Token::LPAREN)
                    {
                        ++lvl;
                    }
                    else if (toks[j].type == Token::RPAREN)
                    {
                        --lvl;
                        if (lvl == 0)
                        {
                            break;
                        }
                    }
                }

                // Parse function and push onto tree.
                Ref<ExpressionParse::MutNode> funcNode;
                TokenIterator funcIt((toks.begin() + i),
                                     (toks.begin() + j + 1));
                const Result res =
                    ExpressionParse::parseFunctionCall(funcIt, funcNode, kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
                nodes.push(funcNode);

                // Jump past the function call.
                i = (j + 1);
                continue;
            }
            else
            {
                // Token is a variable or constant.
                nodes.push(Ref<ExpressionParse::MutNode>(
                    new ExpressionParse::MutNode{tok,
                                                 nullptr,
                                                 nullptr,
                                                 false}));
            }
        }
        else if (tok.type == Token::OPERATOR)
        {
            // Get operator info.
            SF_ASSERT(tok.opInfo != nullptr);
            const OpInfo& opInfo = *tok.opInfo;

            while (!stack.empty())
            {
                // Look up operator info of last item on stack.
                const Token& tokLast = stack.top();
                if (tokLast.type != Token::OPERATOR)
                {
                    // Last item on stack is not an operator; keep going.
                    break;
                }
                SF_ASSERT(tokLast.opInfo != nullptr);
                const OpInfo& lastOpInfo = *tokLast.opInfo;

                // Determine whether to process the subexpression currently on
                // the stack based on the precedence and associativity of this
                // operator and the last. Unary operators are left-associative,
                // and all others are right-associative.
                const bool leftAssoc = (lastOpInfo.unary && opInfo.unary);
                const bool pop =
                    (leftAssoc
                     ? (lastOpInfo.precedence > opInfo.precedence)
                     : (lastOpInfo.precedence >= opInfo.precedence));

                if (pop)
                {
                    // This operator is lower precedence than the last one; add
                    // last operator subexpression onto the expression tree.
                    const Result res =
                        ExpressionParse::popSubexpression(stack, nodes, kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                }
                else
                {
                    // Last operator is lower precedence than this one; keep
                    // going.
                    break;
                }
            }

            // Push operator onto stack.
            stack.push(tok);
        }
        else
        {
            // Token is a right parenthese, indicating the end of a
            // subexpression.

            // Process subexpression on stack.
            while (stack.top().type != Token::LPAREN)
            {
                const Result res =
                    ExpressionParse::popSubexpression(stack, nodes, kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
            }

            // Pop left parenthese.
            SF_ASSERT(stack.top().type == Token::LPAREN);
            stack.pop();
        }

        ++i;
    }

    // Check that stack is empty.
    if (stack.size() != 0)
    {
        ConfigUtil::setError(kErr, stack.top(), gErrText,
                             "invalid expression");
        return E_EXP_SYNTAX;
    }

    // Check that expression tree contains at least 1 node.
    if (nodes.size() == 0)
    {
        ConfigUtil::setError(kErr, kIt[0], gErrText, "invalid expression");
        return E_EXP_EMPTY;
    }

    // Check that there is exactly 1 node on the stack (root node).
    if (nodes.size() != 1)
    {
        ConfigUtil::setError(kErr, nodes.top()->data, gErrText,
                             "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // Expand any double inequalities in the expression.
    ExpressionParse::expandDoubleIneq(nodes.top());

    // Return the root node.
    kNode = nodes.top();

    return SUCCESS;
}

void ExpressionParse::convertTree(Ref<ExpressionParse::MutNode> kFrom,
                                  Ref<const ExpressionParse>& kTo)
{
    if (kFrom == nullptr)
    {
        return;
    }

    Ref<const ExpressionParse> left;
    Ref<const ExpressionParse> right;
    ExpressionParse::convertTree(kFrom->left, left);
    ExpressionParse::convertTree(kFrom->right, right);

    kTo.reset(new ExpressionParse(kFrom->data,
                                  left,
                                  right,
                                  kFrom->func));
}

ExpressionParse::ExpressionParse(const Token kData,
                                 const Ref<const ExpressionParse> kLeft,
                                 const Ref<const ExpressionParse> kRight,
                                 const bool kFunc) :
    data(kData), left(kLeft), right(kRight), func(kFunc)
{
}
