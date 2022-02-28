#include <stack>

#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"

Result ExpressionParser::parse(TokenIterator& kIt,
                               Parse& kParse,
                               ConfigErrorInfo* kConfigErr)
{
    (void) kConfigErr; // rm later

    // Copy token sequence minus newlines into a vector of tokens enclosed in
    // parentheses.
    std::vector<Token> toks = {{Token::LPAREN, "(", -1, -1}};
    while (!kIt.eof())
    {
        toks.push_back(kIt.take());
    }
    toks.push_back({Token::RPAREN, ")", -1, -1});

    std::stack<Parse*> nodes;
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
                // Unknown operator.
                return E_EXP_OP;
            }
            const OperatorInfo& opInfo = (*opInfoIt).second;

            while (true)
            {
                if (stack.empty())
                {
                    // No operands on stack; keep going.
                    break;
                }

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
                    // Unknown operator.
                    return E_EXP_OP;
                }
                const OperatorInfo& lastOpInfo = (*lastOpInfoIt).second;

                if (lastOpInfo.precedence >= opInfo.precedence)
                {
                    // Last operator is higher predecence than this one; pop
                    // operator, operands, and push onto expression.

                    // Pop operator off stack.
                    const Token& op = stack.top();
                    stack.pop();

                    // Pop RHS off stack.
                    if (nodes.empty())
                    {
                        // Expected an RHS.
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
                    return E_EXP_SYNTAX;
                }

                // Look up operator info.
                auto opInfoIt = OperatorInfo::fromStr.find(op.str);
                if (opInfoIt == OperatorInfo::fromStr.end())
                {
                    // Unknown operator.
                    return E_EXP_OP;
                }
                const OperatorInfo& opInfo = (*opInfoIt).second;

                // Pop RHS from stack.
                if (nodes.empty())
                {
                    // Expected an RHS.
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
            return E_EXP_TOK;
        }
    }

    kParse = *nodes.top();
    delete nodes.top();

    return SUCCESS;
}
