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
    /// identifier, not the nodes therein), false otherwise.
    ///
    bool func;

private:

    friend class ExpressionParser;

    ///
    /// @brief Mutable tree node to aid tree construction.
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
    /// @param[in] kIt     Iterator of token sequence to parse.
    /// @param[in] kParse  On SUCCESS, points to parsed expression.
    /// @param[in] kErr    On error, contains error info.
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

    static Result popSubexpression(
        std::stack<Token>& kStack,
        std::stack<Ref<ExpressionParse::MutNode>>& kNodes,
        ErrorInfo* const kErr);

    static Result parseFunctionCall(TokenIterator kIt,
                                    Ref<ExpressionParse::MutNode>& kNode,
                                    ErrorInfo* const kErr);

    static void expandDoubleIneq(const Ref<ExpressionParse::MutNode> kNode);

    static Result parseImpl(TokenIterator& kIt,
                            Ref<ExpressionParse::MutNode>& kParse,
                            ErrorInfo* const kErr);

    static void convertTree(Ref<ExpressionParse::MutNode> kFrom,
                            Ref<const ExpressionParse>& kTo);
};

#endif
