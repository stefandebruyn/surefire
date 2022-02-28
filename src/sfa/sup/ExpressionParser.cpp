#include <stack>

#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"

static const char* const gErrText = "expression error";

Result ExpressionParser::parse(TokenIterator& kIt,
                               Parse& kParse,
                               ConfigErrorInfo* kConfigErr)
{
    (void) kConfigErr; // rm later

    // Check that token sequence is not empty.
    if (kIt.size() == 0)
    {
        return E_EXP_EMPTY;
    }

    // Check that parentheses are balanced.
    TokenIterator itCpy = kIt;
    I32 lvl = 0;
    const Token* tokLastParen = nullptr;
    while (!itCpy.eof())
    {
        if (itCpy.type() == Token::LPAREN)
        {
            // Save parenthese token in case we need it for an error message.
            tokLastParen = &itCpy.tok();
            ++lvl;
        }
        else if (itCpy.type() == Token::RPAREN)
        {
            // Save parenthese token in case we need it for an error message.
            tokLastParen = &itCpy.tok();
            --lvl;

            if (lvl < 0)
            {
                // Unbalanced parentheses.
                ConfigUtil::setError(kConfigErr, itCpy.tok(), gErrText,
                                     "unbalanced parenthese");
                return E_EXP_PAREN;
            }
        }

        itCpy.take();
    }
    if (lvl != 0)
    {
        // Unbalanced parentheses.
        SFA_ASSERT(tokLastParen != nullptr);
        ConfigUtil::setError(kConfigErr, *tokLastParen, gErrText,
                             "unbalanced parenthese");
        return E_EXP_PAREN;
    }

    // Copy token sequence, minus newlines, into a vector of tokens enclosed in
    // parentheses.
    std::vector<Token> toks = {{Token::LPAREN, "(", -1, -1}};
    while (!kIt.eof())
    {
        toks.push_back(kIt.take());
    }
    toks.push_back({Token::RPAREN, ")", -1, -1});

    // Stack of expression nodes yet to be installed in the binary tree.
    std::stack<Parse*> nodes;

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
                nodes.push(new Parse{tok, nullptr, nullptr});
            }
        }
        else if (tok.type == Token::OPERATOR)
        {
            // Look up operator info.
            auto opInfoIt = OperatorInfo::fromStr.find(tok.str);
            if (opInfoIt == OperatorInfo::fromStr.end())
            {
                // Unknown operator. This would indicate a missing key in
                // `OperatorInfo::fromStr`, so provide no config error message.
                return E_EXP_OP;
            }
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
                if (lastOpInfoIt == OperatorInfo::fromStr.end())
                {
                    // Unknown operator. This would indicate a missing key in
                    // `OperatorInfo::fromStr`, so provide no config error
                    // message.
                    return E_EXP_OP;
                }
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
                        ConfigUtil::setError(kConfigErr, op, gErrText,
                                             "invalid syntax");
                        return E_EXP_SYNTAX;
                    }
                    Parse* const right = nodes.top();
                    nodes.pop();

                    Parse* left = nullptr;
                    if (!opInfo.unary)
                    {
                        // Pop LHS off stack.
                        if (nodes.empty())
                        {
                            // Expected an LHS.
                            ConfigUtil::setError(kConfigErr, op, gErrText,
                                                 "invalid syntax");
                            return E_EXP_SYNTAX;
                        }
                        left = nodes.top();
                        nodes.pop();
                    }

                    // Push operation onto expression.
                    nodes.push(new Parse{op, left, right});
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
        else if (tok.type == Token::RPAREN)
        {
            while (!stack.empty() && (stack.top().type != Token::LPAREN))
            {
                // Pop operator off stack.
                const Token& op = stack.top();
                stack.pop();
                if (op.type != Token::OPERATOR)
                {
                    // Expected an operator.
                    ConfigUtil::setError(kConfigErr, op, gErrText,
                                         "invalid syntax");
                    return E_EXP_SYNTAX;
                }

                // Look up operator info.
                auto opInfoIt = OperatorInfo::fromStr.find(op.str);
                if (opInfoIt == OperatorInfo::fromStr.end())
                {
                    // Unknown operator. This would indicate a missing key in
                    // `OperatorInfo::fromStr`, so provide no config error
                    // message.
                    return E_EXP_OP;
                }
                const OperatorInfo& opInfo = (*opInfoIt).second;

                // Pop RHS from stack.
                if (nodes.empty())
                {
                    // Expected an RHS.
                    ConfigUtil::setError(kConfigErr, op, gErrText,
                                         "invalid syntax");
                    return E_EXP_SYNTAX;
                }
                Parse* const right = nodes.top();
                nodes.pop();

                Parse* left = nullptr;
                if (!opInfo.unary)
                {
                    // Pop LHS from stack.
                    if (nodes.empty())
                    {
                        // Expected an LHS.
                        ConfigUtil::setError(kConfigErr, op, gErrText,
                                             "invalid syntax");
                        return E_EXP_SYNTAX;
                    }
                    left = nodes.top();
                    nodes.pop();
                }

                // Push operation onto expression.
                nodes.push(new Parse{op, left, right});
            }

            if (!stack.empty())
            {
                stack.pop();
            }
        }
        else
        {
            // Unexpected token in expression.
            ConfigUtil::setError(kConfigErr, tok, gErrText,
                                 "unexpected token in expression");
            return E_EXP_TOK;
        }
    }

    if (nodes.size() == 0)
    {
        // Expression contains no terms.
        ConfigUtil::setError(kConfigErr, kIt[0], gErrText,
                             "invalid expression");
        return E_EXP_EMPTY;
    }

    // Assert that there's exactly one node on the node stack. This is the root
    // node of the expression binary tree.
    SFA_ASSERT(nodes.size() == 1);

    // Manually move root node into the caller-provided node.
    kParse = *nodes.top();
    delete nodes.top();

    return SUCCESS;
}
