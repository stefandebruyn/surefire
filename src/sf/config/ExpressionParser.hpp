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
/// @file  sf/config/ExpressionParser.hpp
/// @brief Parser for expressions in config files.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_EXPRESSION_PARSER_HPP
#define SF_EXPRESSION_PARSER_HPP

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/config/Tokenizer.hpp"

///
/// @brief Parsed expression. Expressions are represented as trees.
///
class ExpressionParse final
{
public:

    ///
    /// @brief Expression tree node token.
    ///
    Token data;

    ///
    /// @brief Pointer to left subtree, or null if none.
    ///
    Ref<const ExpressionParse> left;

    ///
    /// @brief Pointer to riht subtree, or null if none.
    ///
    Ref<const ExpressionParse> right;

    ///
    /// @brief True if this node is a function call (just the initial function
    /// identifier, not the descendant nodes), false otherwise.
    ///
    bool func;

private:

    friend class ExpressionParser;

    ///
    /// @brief Mutable tree node to make in-place tree mutations easier during
    /// parsing.
    ///
    struct MutNode final
    {
        Token data;                          ///< Node token.
        Ref<ExpressionParse::MutNode> left;  ///< Left subtree.
        Ref<ExpressionParse::MutNode> right; ///< Right subtree.
        bool func;                           ///< If node is a function call.
    };

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kData   Node token.
    /// @param[in] kLeft   Left subtree.
    /// @param[in] kRight  Right subtree.
    /// @param[in] kFunc   If node is a function call.
    ///
    ExpressionParse(const Token kData,
                    const Ref<const ExpressionParse> kLeft,
                    const Ref<const ExpressionParse> kRight,
                    const bool kFunc);
};

///
/// @brief Expression parser.
///
class ExpressionParser final
{
public:

    ///
    /// @brief Parsing entry point.
    ///
    /// @param[in]  kIt     Iterator of token sequence to parse.
    /// @param[out] kParse  On success, points to parsed expression.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @retval SUCCESS       Successfully parsed expression.
    /// @retval E_EXP_EMPTY   Token sequence is empty.
    /// @retval E_EXP_PAREN   Expression has unbalanced parentheses.
    /// @retval E_EXP_OP      Expression contains an illegal operator.
    /// @retval E_EXP_TOK     Expression contains an illegal token.
    /// @retval E_EXP_SYNTAX  Expression contains a syntax error.
    ///
    static Result parse(TokenIterator kIt,
                        Ref<const ExpressionParse>& kParse,
                        ErrorInfo* const kErr);

    ExpressionParser() = delete;

private:

    ///
    /// @brief Pops a subexpression from the operator stack onto the expression
    /// tree according to operator precedence.
    ///
    /// @param[in]  kStack  Stack of nodes to pop from.
    /// @param[out] kNodes  Stack of nodes to push to.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionParser::parse().
    ///
    static Result popSubexpression(
        std::stack<Token>& kStack,
        std::stack<Ref<ExpressionParse::MutNode>>& kNodes,
        ErrorInfo* const kErr);

    ///
    /// @brief Parses a function call. The root of the parsed subtree contains
    /// the function identifier. The left subtree is a linked list of function
    /// argument parses, where the left subtree of each argument is the next
    /// argument, and the right subtree is the root of the argument expression.
    /// For example, `foo(a, b + c)` becomes:
    ///
    ///       foo
    ///      /
    ///     arg1
    ///    /    \
    ///   arg2   a
    ///       \
    ///        +
    ///       / \
    ///      b   c
    ///
    /// @param[in]  kIt    Token iterator positioned at function call.
    /// @param[out] kNode  On success, contains function call parse.
    /// @param[out] kErr   On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionParser::parse().
    ///
    static Result parseFunctionCall(TokenIterator kIt,
                                    Ref<ExpressionParse::MutNode>& kNode,
                                    ErrorInfo* const kErr);

    ///
    /// @brief Recursively expands double inequalities in an expression by
    /// expanding them into single inequalities joined by logical ANDs. For
    /// example, `a < b < c` becomes `a < b and b < c`.
    ///
    /// @param[in, out] kNode  Root of expression subtree to start at.
    ///
    static void expandDoubleIneq(const Ref<ExpressionParse::MutNode> kNode);

    ///
    /// @brief Main parsing method.
    ///
    /// @param[in]  kIt     Iterator of token sequence to parse.
    /// @param[out] kParse  On success, points to parsed expression.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    static Result parseImpl(TokenIterator& kIt,
                            Ref<ExpressionParse::MutNode>& kParse,
                            ErrorInfo* const kErr);

    ///
    /// @brief Recursively converts a mutable expression tree to an immutable
    /// one.
    ///
    /// @param[in]  kFrom  Root of mutable tree.
    /// @param[out] kTo    Root of immutable tree.
    ///
    static void convertTree(Ref<ExpressionParse::MutNode> kFrom,
                            Ref<const ExpressionParse>& kTo);
};

#endif
