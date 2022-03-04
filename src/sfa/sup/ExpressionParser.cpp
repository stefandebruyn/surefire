#include <stack>

#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"

/////////////////////////////////// Private ////////////////////////////////////

// Private namespace members.
namespace ExpressionParser
{
    const char* const errText = "error";

    Result popSubexpression(std::stack<Token>& kStack,
                            std::stack<std::shared_ptr<Parse>>& kNodes,
                            ConfigErrorInfo* kConfigErr);

    Result parseFunctionCall(TokenIterator kIt,
                             std::shared_ptr<Parse>& kParse,
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

Result ExpressionParser::parseFunctionCall(TokenIterator kIt,
                                           std::shared_ptr<Parse>& kParse,
                                           ConfigErrorInfo* kConfigErr)
{
    // Assert that token sequence is an identifier followed by an open
    // parenthese and ending with a close parenthese.
    SFA_ASSERT(kIt.size() >= 3);
    SFA_ASSERT(kIt[0].type == Token::IDENTIFIER);
    SFA_ASSERT(kIt[1].type == Token::LPAREN);
    SFA_ASSERT(kIt[kIt.size() - 1].type == Token::RPAREN);

    // Stores iterators for each argument expression in the function call.
    std::vector<TokenIterator> argExprs;
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
                ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
    kParse.reset(new Parse{kIt[0], nullptr, nullptr, true});

    // Parse argument expressions and chain them down the left subtree of
    // the function call node. The left child of each argument node is the next
    // argument, and the right child is the argument expression.
    std::shared_ptr<Parse> node = kParse;
    for (TokenIterator& argIt : argExprs)
    {
        node->left.reset(new Parse{{}, nullptr, nullptr, false});
        const Result res = parseImpl(argIt, node->left->right, kConfigErr);
        if (res != SUCCESS)
        {
            return res;
        }
        node = node->left;
    }

    return SUCCESS;
}

void ExpressionParser::expandDoubleInequalities(std::shared_ptr<Parse> kNode)
{
    // Recursion base case.
    if (kNode == nullptr)
    {
        return;
    }

    if ((kNode->left != nullptr) && (kNode->right != nullptr))
    {
        if (OperatorInfo::relOps.find(kNode->data.str)
            != OperatorInfo::relOps.end())
        {
            // If this node and the left node contain a relational operator,
            // this is a double inequality. It's impossible for the right node
            // to contain a relational operator since all operators used in
            // double inequalities have the same precedence and are
            // right-associative.
            if (OperatorInfo::relOps.find(kNode->left->data.str)
                != OperatorInfo::relOps.end())
            {
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
                std::shared_ptr<Parse> funcNode;
                TokenIterator funcIt((toks.begin() + i),
                                     (toks.begin() + j + 1));
                const Result res = parseFunctionCall(funcIt,
                                                     funcNode,
                                                     kConfigErr);
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

        ++i;
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

Result ExpressionParser::parse(TokenIterator kIt,
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
                ConfigUtil::setError(kConfigErr, tok, errText,
                                     "unbalanced parenthese");
                return E_EXP_PAREN;
            }
        }
    }

    if (lvl != 0)
    {
        // Unbalanced parentheses.
        SFA_ASSERT(tokLastLvl0Paren != nullptr);
        ConfigUtil::setError(kConfigErr, *tokLastLvl0Paren, errText,
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