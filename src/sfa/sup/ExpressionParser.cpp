#include <stack>

#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace ExpressionParser
{
    const char* const errText = "expression error";

    Result parseImpl(TokenIterator& kIt,
                     std::shared_ptr<Parse>& kParse,
                     ConfigErrorInfo* kConfigErr);
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
                nodes.push(
                    std::shared_ptr<Parse>(new Parse{tok, nullptr, nullptr}));
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

                if (lastOpInfo.precedence >= opInfo.precedence)
                {
                    // This operator is lower precedence than the last one; add
                    // last one onto the expression tree.

                    // Pop operator off stack.
                    const Token& op = stack.top();
                    stack.pop();

                    // Pop RHS off stack.
                    if (nodes.empty())
                    {
                        // Expected an RHS.
                        ConfigUtil::setError(kConfigErr, op, errText,
                                             "invalid syntax");
                        return E_EXP_SYNTAX;
                    }
                    const std::shared_ptr<Parse> right = nodes.top();
                    nodes.pop();

                    std::shared_ptr<Parse> left;
                    if (!opInfo.unary)
                    {
                        // Pop LHS off stack.
                        if (nodes.empty())
                        {
                            // Expected an LHS.
                            ConfigUtil::setError(kConfigErr, op, errText,
                                                 "invalid syntax");
                            return E_EXP_SYNTAX;
                        }
                        left = nodes.top();
                        nodes.pop();
                    }

                    // Push operation onto expression.
                    nodes.push(
                        std::shared_ptr<Parse>(new Parse{op, left, right}));
                }
                else
                {
                    // Last operator is lower precedence than this one; keep
                    // going.
                    break;
                }
            }

            stack.push(tok);
        }
        else
        {
            // Token is a right parenthese, indicating the end of a
            // subexpression.

            // Process subexpression on stack.
            while (!stack.empty() && (stack.top().type != Token::LPAREN))
            {
                // Pop operator off stack.
                const Token& op = stack.top();
                stack.pop();
                if (op.type != Token::OPERATOR)
                {
                    // Expected an operator.
                    ConfigUtil::setError(kConfigErr, op, errText,
                                         "invalid syntax");
                    return E_EXP_SYNTAX;
                }

                // Look up operator info.
                auto opInfoIt = OperatorInfo::fromStr.find(op.str);
                const OperatorInfo& opInfo = (*opInfoIt).second;

                // Pop RHS from stack.
                if (nodes.empty())
                {
                    // Expected an RHS.
                    ConfigUtil::setError(kConfigErr, op, errText,
                                         "invalid syntax");
                    return E_EXP_SYNTAX;
                }
                const std::shared_ptr<Parse> right = nodes.top();
                nodes.pop();

                // Check that RHS comes before the operator in the expression.
                if ((right->data.lineNum < op.lineNum)
                    || (right->data.colNum < op.colNum))
                {
                    // "RHS" is actually to the left of the operator. This
                    // usually indicates a syntax error with a unary operator.
                    ConfigUtil::setError(kConfigErr, op, errText,
                                         "invalid syntax");
                    return E_EXP_SYNTAX;
                }

                std::shared_ptr<Parse> left;
                if (!opInfo.unary)
                {
                    // Pop LHS from stack.
                    if (nodes.empty())
                    {
                        // Expected an LHS.
                        ConfigUtil::setError(kConfigErr, op, errText,
                                             "invalid syntax");
                        return E_EXP_SYNTAX;
                    }
                    left = nodes.top();
                    nodes.pop();
                }

                // Push operation onto expression.
                nodes.push(std::shared_ptr<Parse>(new Parse{op, left, right}));
            }

            if (!stack.empty())
            {
                stack.pop();
            }
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
