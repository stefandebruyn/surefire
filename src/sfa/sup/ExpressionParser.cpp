#include <stack>

#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace ExpressionParser
{
    const char* const errText = "expression error";

    Result popSubexpression(std::stack<Token>& kStack,
                            std::stack<std::shared_ptr<Parse>>& kNodes,
                            ConfigErrorInfo* kConfigErr);

    void expandDoubleInequalities(std::shared_ptr<Parse> kParse);

    Result parseImpl(TokenIterator& kIt,
                     std::shared_ptr<Parse>& kParse,
                     ConfigErrorInfo* kConfigErr);
}

Result ExpressionParser::popSubexpression(
    std::stack<Token>& kStack,
    std::stack<std::shared_ptr<Parse>>& kNodes,
    ConfigErrorInfo* kConfigErr)
{
    // Pop operator off stack.
    const Token& op = kStack.top();
    kStack.pop();
    if (op.type != Token::OPERATOR)
    {
        // Expected an operator.
        ConfigUtil::setError(kConfigErr, op, errText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // Look up operator info.
    auto opInfoIt = OperatorInfo::fromStr.find(op.str);
    SFA_ASSERT(opInfoIt != OperatorInfo::fromStr.end());
    const OperatorInfo& opInfo = (*opInfoIt).second;

    // Pop RHS from stack.
    if (kNodes.empty())
    {
        // Expected an RHS.
        ConfigUtil::setError(kConfigErr, op, errText, "invalid syntax");
        return E_EXP_SYNTAX;
    }
    const std::shared_ptr<Parse> right = kNodes.top();
    kNodes.pop();

    // Check that RHS comes before the operator in the expression.
    if ((right->data.lineNum < op.lineNum) || (right->data.colNum < op.colNum))
    {
        // "RHS" is actually to the left of the operator. This
        // usually indicates a syntax error with a unary operator.
        ConfigUtil::setError(kConfigErr, op, errText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    std::shared_ptr<Parse> left;
    if (!opInfo.unary)
    {
        // Pop LHS from stack.
        if (kNodes.empty())
        {
            // Expected an LHS.
            ConfigUtil::setError(kConfigErr, op, errText, "invalid syntax");
            return E_EXP_SYNTAX;
        }
        left = kNodes.top();
        kNodes.pop();
    }

    // Push operation onto expression.
    kNodes.push(std::shared_ptr<Parse>(new Parse{op, left, right, false}));

    return SUCCESS;
}

void ExpressionParser::expandDoubleInequalities(std::shared_ptr<Parse> kNode)
{
    // Base case: we've fallen off the expression tree.
    if (kNode == nullptr)
    {
        return;
    }

    if ((kNode->left != nullptr) && (kNode->right != nullptr))
    {
        if (OperatorInfo::relOps.find(kNode->data.str)
            != OperatorInfo::relOps.end())
        {
            // If this node contains a relational operator and the left node
            // does as well, this is a double inequality. It's impossible for
            // the right node to contain a relational operator since the ones
            // used in double inequalities have the same precedence and are
            // right-associative.
            if (OperatorInfo::relOps.find(kNode->left->data.str)
                != OperatorInfo::relOps.end())
            {
                // Node is the right operator in a double inequality.
                kNode->right.reset(new Parse{kNode->data,
                                             kNode->left->right,
                                             kNode->right,
                                             false});
                kNode->data = {Token::OPERATOR, "AND", -1, -1};
            }
        }
    }

    // Recurse into left and right subtrees.
    expandDoubleInequalities(kNode->left);
    expandDoubleInequalities(kNode->right);
}

Result ExpressionParser::parseImpl(TokenIterator& kIt,
                                   std::shared_ptr<Parse>& kParse,
                                   ConfigErrorInfo* kConfigErr)
{
    // Copy token sequence, minus newlines, into a vector of tokens enclosed in
    // parentheses. Adding this extra pair of parentheses simplifies the
    // algorithm.
    std::vector<Token> toks = {{Token::LPAREN, "(", -1, -1}};
    kIt.seek(0);
    while (!kIt.eof())
    {
        toks.push_back(kIt.take());
    }
    toks.push_back({Token::RPAREN, ")", -1, -1});

    // Stack of expression nodes yet to be installed in the binary tree.
    std::stack<std::shared_ptr<Parse>> nodes;

    // Operator and operand stack.
    std::stack<Token> stack;

    for (std::size_t i = 0; i < toks.size(); ++i)
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
                SFA_ASSERT(false);
            }
            else
            {
                // Token is a variable or constant.
                nodes.push(std::shared_ptr<Parse>(
                    new Parse{tok, nullptr, nullptr, false}));
            }
        }
        else if (tok.type == Token::OPERATOR)
        {
            // Look up operator info.
            auto opInfoIt = OperatorInfo::fromStr.find(tok.str);
            const OperatorInfo& opInfo = (*opInfoIt).second;

            while (!stack.empty())
            {
                // Look up operator info of last item on stack.
                const Token& tokLast = stack.top();
                if (tokLast.type != Token::OPERATOR)
                {
                    // Last item on stack is not an operator; keep going.
                    break;
                }
                auto lastOpInfoIt = OperatorInfo::fromStr.find(tokLast.str);
                const OperatorInfo& lastOpInfo = (*lastOpInfoIt).second;

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
                    const Result res = popSubexpression(stack,
                                                        nodes,
                                                        kConfigErr);
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
                const Result res = popSubexpression(stack, nodes, kConfigErr);
                if (res != SUCCESS)
                {
                    return res;
                }
            }

            // Pop left parenthese.
            SFA_ASSERT(stack.top().type == Token::LPAREN);
            stack.pop();
        }
    }

    // Check that stack is empty.
    if (stack.size() != 0)
    {
        ConfigUtil::setError(kConfigErr, stack.top(), errText,
                             "invalid expression");
        return E_EXP_SYNTAX;
    }

    // Check that expression tree contains at least 1 node.
    if (nodes.size() == 0)
    {
        ConfigUtil::setError(kConfigErr, kIt[0], errText, "invalid expression");
        return E_EXP_EMPTY;
    }

    // Check that there is exactly 1 node on the stack (root node).
    if (nodes.size() != 1)
    {
        // Expression contains some kind of invalid syntax, probably a missing
        // operator.
        ConfigUtil::setError(kConfigErr, nodes.top()->data, errText,
                             "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // If we got this far, successfully parsed the expression. Move root node
    // into return pointer.
    kParse = nodes.top();

    // Expand any double inequalities in the expression.
    expandDoubleInequalities(kParse);

    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

Result ExpressionParser::parse(TokenIterator& kIt,
                               std::shared_ptr<Parse>& kParse,
                               ConfigErrorInfo* kConfigErr)
{
    // Assert that the iterator is at index zero.
    SFA_ASSERT(kIt.idx() == 0);

    // Check that token sequence is not empty.
    if (kIt.size() == 0)
    {
        return E_EXP_EMPTY;
    }

    // Check that parentheses are balanced.
    I32 lvl = 0;
    const Token* tokLastParen = nullptr;
    while (!kIt.eof())
    {
        const Token& tok = kIt.take();
        if (tok.type == Token::LPAREN)
        {
            // Save parenthese token in case we need it for an error message.
            tokLastParen = &tok;
            ++lvl;
        }
        else if (tok.type == Token::RPAREN)
        {
            // Save parenthese token in case we need it for an error message.
            tokLastParen = &tok;
            --lvl;

            if (lvl < 0)
            {
                // Unbalanced parentheses.
                ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                     "unbalanced parenthese");
                return E_EXP_PAREN;
            }
        }
    }

    if (lvl != 0)
    {
        // Unbalanced parentheses.
        SFA_ASSERT(tokLastParen != nullptr);
        ConfigUtil::setError(kConfigErr, *tokLastParen, errText,
                             "unbalanced parenthese");
        return E_EXP_PAREN;
    }

    // Check that all operator tokens have operator info available.
    kIt.seek(0);
    while (!kIt.eof())
    {
        const Token& tok = kIt.take();
        if (tok.type == Token::OPERATOR)
        {
            auto opInfoIt = OperatorInfo::fromStr.find(tok.str);
            if (opInfoIt == OperatorInfo::fromStr.end())
            {
                // No info for operator. This would indicate a bug in the
                // framework and not an error on the user's part, so provide
                // no config error message.
                return E_EXP_OP;
            }
        }
    }

    // Check that expression contains only identifier, constant, operator,
    // and parenthese tokens.
    kIt.seek(0);
    while (!kIt.eof())
    {
        const Token& tok = kIt.take();
        if ((tok.type != Token::IDENTIFIER)
            && (tok.type != Token::CONSTANT)
            && (tok.type != Token::OPERATOR)
            && (tok.type != Token::LPAREN)
            && (tok.type != Token::RPAREN))
        {
            // Unexpected token in expression.
            ConfigUtil::setError(kConfigErr, tok, errText,
                                 "unexpected token in expression");
            return E_EXP_TOK;
        }
    }

    // At this point we know that the expression has balanced parentheses,
    // contains only known operators, and contains no unexpected token types.
    // So, the only errors to check for herein are syntax errors.

    return parseImpl(kIt, kParse, kConfigErr);
}
