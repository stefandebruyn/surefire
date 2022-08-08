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
////////////////////////////////////////////////////////////////////////////////

#include <stack>

#include "sf/config/ExpressionParser.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Global /////////////////////////////////////

///
/// @brief Expression parser error text.
///
static const char* const gErrText = "expression error";

/////////////////////////////////// Public /////////////////////////////////////

Result ExpressionParser::parse(TokenIterator kIt,
                               Ref<const ExpressionParse>& kParse,
                               ErrorInfo* const kErr)
{
    // Assert that iterator is at the start of the expression.
    SF_SAFE_ASSERT(kIt.idx() == 0);

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
                ErrorInfo::set(kErr, tok, gErrText, "unbalanced parenthese");
                return E_EXP_PAREN;
            }
        }
    }

    // Check that parentheses are balanced.
    if (lvl != 0)
    {
        SF_SAFE_ASSERT(tokLastLvl0Paren != nullptr);
        ErrorInfo::set(kErr, *tokLastLvl0Paren, gErrText,
                       "unbalanced parenthese");
        return E_EXP_PAREN;
    }

    // Check that expression contains no assignment operators, which is the one
    // operator that may not appear in expressions.
    kIt.seek(0);
    while (!kIt.eof())
    {
        const Token& tok = kIt.take();
        if (tok.str == "=")
        {
            ErrorInfo::set(
                kErr, tok, gErrText,
                "invalid operator in expression (did you mean `==`?)");
            return E_EXP_OP;
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
            ErrorInfo::set(kErr, tok, gErrText,
                           "unexpected token in expression");
            return E_EXP_TOK;
        }
    }

    // At this point we know that the expression has balanced parentheses,
    // contains only known operators, and contains no unexpected token types.
    // So, the only errors to check for herein are syntax errors.
    Ref<ExpressionParse::MutNode> root;
    const Result res = ExpressionParser::parseImpl(kIt, root, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Convert tree to the public, const type.
    ExpressionParser::convertTree(root, kParse);

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

Result ExpressionParser::popSubexpression(
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
        ErrorInfo::set(kErr, op, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // Get operator info.
    SF_SAFE_ASSERT(op.opInfo != nullptr);
    const OpInfo& opInfo = *op.opInfo;

    // Pop RHS from stack.
    if (kNodes.empty())
    {
        // Expected an RHS.
        ErrorInfo::set(kErr, op, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }
    const Ref<ExpressionParse::MutNode> right = kNodes.top();
    kNodes.pop();

    // Check that RHS comes before the operator in the expression.
    SF_SAFE_ASSERT(right != nullptr);
    if ((right->data.lineNum < op.lineNum) || (right->data.colNum < op.colNum))
    {
        // "RHS" is actually to the left of the operator. This usually indicates
        // a syntax error with a unary operator.
        ErrorInfo::set(kErr, op, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    Ref<ExpressionParse::MutNode> left;
    if (!opInfo.unary)
    {
        // Pop LHS from stack.
        if (kNodes.empty())
        {
            // Expected an LHS.
            ErrorInfo::set(kErr, op, gErrText, "invalid syntax");
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

Result ExpressionParser::parseFunctionCall(TokenIterator kIt,
                                           Ref<ExpressionParse::MutNode>& kNode,
                                           ErrorInfo* const kErr)
{
    // Assert that token sequence is an identifier followed by an open
    // parenthese and ending with a close parenthese.
    SF_SAFE_ASSERT(kIt.size() >= 3);
    SF_SAFE_ASSERT(kIt[0].type == Token::IDENTIFIER);
    SF_SAFE_ASSERT(kIt[1].type == Token::LPAREN);
    SF_SAFE_ASSERT(kIt[kIt.size() - 1].type == Token::RPAREN);

    // Vector of iterators for each argument expression in the function call.
    Vec<TokenIterator> argExprs;

    // Parenthese level.
    U32 lvl = 0;

    // Start parsing at index 2, the first token after the open parenthese.
    U32 idxArgStart = 2;
    kIt.seek(idxArgStart);

    while (!kIt.eof())
    {
        // Increment parenthese level on open parenthese.
        if (kIt.type() == Token::LPAREN)
        {
            ++lvl;
        }

        // Decrement parenthese level on close parenthese. Ignore the final
        // close parenthese, which is a special case that marks the end of the
        // last function argument.
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
                ErrorInfo::set(kErr, kIt.tok(), gErrText, "invalid syntax");
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

        // Go to next token.
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
        node->left.reset(new ExpressionParse::MutNode{TokenIterator::NONE,
                                                      nullptr,
                                                      nullptr,
                                                      false});
        const Result res = ExpressionParser::parseImpl(argIt,
                                                       node->left->right,
                                                       kErr);
        if (res != SUCCESS)
        {
            return res;
        }
        node = node->left;
    }

    return SUCCESS;
}

void ExpressionParser::expandDoubleIneq(
    const Ref<ExpressionParse::MutNode> kNode)
{
    // Base case: node is null, so we fell off the tree.
    if (kNode == nullptr)
    {
        return;
    }

    // Skip nodes without left and right subtrees (which can't be operators).
    if ((kNode->left != nullptr) && (kNode->right != nullptr))
    {
        if (OpInfo::relOps.find(kNode->data.str) != OpInfo::relOps.end())
        {
            // If this node and the left node contain a relational operator,
            // this is a double inequality. It's impossible for the right node
            // to contain a relational operator since all operators used in
            // double inequalities have the same precedence and are
            // right-associative.
            if (OpInfo::relOps.find(kNode->left->data.str)
                != OpInfo::relOps.end())
            {
                // Join inequalities with an AND operator.
                kNode->right.reset(
                    new ExpressionParse::MutNode{kNode->data,
                                                 kNode->left->right,
                                                 kNode->right,
                                                 false});
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
    ExpressionParser::expandDoubleIneq(kNode->left);
    ExpressionParser::expandDoubleIneq(kNode->right);
}

Result ExpressionParser::parseImpl(TokenIterator& kIt,
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

    // Stack of expression tree nodes.
    std::stack<Ref<ExpressionParse::MutNode>> nodes;

    // Stack of nodes not yet installed in the expression tree.
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

                // Assert that parenthese level is zero since balanced
                // parentheses was validated previously.
                SF_SAFE_ASSERT(lvl == 0);

                // Parse function and push onto tree.
                Ref<ExpressionParse::MutNode> funcNode;
                TokenIterator funcIt((toks.begin() + i),
                                     (toks.begin() + j + 1));
                const Result res = ExpressionParser::parseFunctionCall(funcIt,
                                                                       funcNode,
                                                                       kErr);
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
            SF_SAFE_ASSERT(tok.opInfo != nullptr);
            const OpInfo& opInfo = *tok.opInfo;

            while (!stack.empty())
            {
                // If last node on stack is not an operator, keep going.
                const Token& tokLast = stack.top();
                if (tokLast.type != Token::OPERATOR)
                {
                    break;
                }

                // Get operator info for last operator.
                SF_SAFE_ASSERT(tokLast.opInfo != nullptr);
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
                    const Result res = ExpressionParser::popSubexpression(stack,
                                                                         nodes,
                                                                         kErr);
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
                const Result res = ExpressionParser::popSubexpression(stack,
                                                                     nodes,
                                                                     kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
            }

            // Pop left parenthese.
            SF_SAFE_ASSERT(stack.top().type == Token::LPAREN);
            stack.pop();
        }

        ++i;
    }

    // Check that stack is empty.
    if (stack.size() != 0)
    {
        ErrorInfo::set(kErr, stack.top(), gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // Check that expression tree contains at least 1 node.
    if (nodes.size() == 0)
    {
        ErrorInfo::set(kErr, kIt[0], gErrText, "invalid syntax");
        return E_EXP_EMPTY;
    }

    // Check that there is exactly 1 node on the stack (root node).
    if (nodes.size() != 1)
    {
        ErrorInfo::set(kErr, nodes.top()->data, gErrText, "invalid syntax");
        return E_EXP_SYNTAX;
    }

    // Expand any double inequalities in the expression.
    ExpressionParser::expandDoubleIneq(nodes.top());

    // Return the root node.
    kNode = nodes.top();

    return SUCCESS;
}

void ExpressionParser::convertTree(Ref<ExpressionParse::MutNode> kFrom,
                                   Ref<const ExpressionParse>& kTo)
{
    // Base case: node is null, so we fell off the tree.
    if (kFrom == nullptr)
    {
        return;
    }

    // Convert left and right subtrees.
    Ref<const ExpressionParse> left;
    Ref<const ExpressionParse> right;
    ExpressionParser::convertTree(kFrom->left, left);
    ExpressionParser::convertTree(kFrom->right, right);

    // Convert current node.
    kTo.reset(new ExpressionParse(kFrom->data, left, right, kFrom->func));
}

ExpressionParse::ExpressionParse(const Token kData,
                                 const Ref<const ExpressionParse> kLeft,
                                 const Ref<const ExpressionParse> kRight,
                                 const bool kFunc) :
    data(kData), left(kLeft), right(kRight), func(kFunc)
{
}
