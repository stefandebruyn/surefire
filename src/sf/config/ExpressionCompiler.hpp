////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
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
/// @file  sf/config/ExpressionCompiler.hpp
/// @brief Compiler for expressions in config files.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_EXPRESSION_COMPILER_HPP
#define SF_EXPRESSION_COMPILER_HPP

#include "sf/config/ExpressionParser.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/ExpressionStats.hpp"
#include "sf/core/StateVector.hpp"

namespace Sf
{

///
/// @brief Compiled expression.
///
class ExpressionAssembly final
{
public:

    ///
    /// @brief Gets the expression root node.
    ///
    /// @returns Expression root node.
    ///
    Ref<IExpression> root() const;

    ///
    /// @brief Gets a vector of expression stats used by the expression, e.g.,
    /// if it uses a function like roll_avg().
    ///
    /// @returns Expression stats vector.
    ///
    Vec<Ref<IExpressionStats>> stats() const;

private:

    friend class ExpressionCompiler;

    ///
    /// @brief Set of data that represents the expression.
    ///
    struct Workspace
    {
        Vec<Ref<IExpression>> exprNodes;
        Vec<Ref<IExpressionStats>> exprStats;
        Vec<Ref<Vec<U8>>> statArrs;
        Ref<IExpression> rootNode;
    };

    ///
    /// @brief Workspace.
    ///
    const ExpressionAssembly::Workspace mWs;

    ///
    /// @brief Contructor.
    ///
    /// @param[in] kWs  Workspace.
    ///
    ExpressionAssembly(const ExpressionAssembly::Workspace& kWs);
};

///
/// @brief Expression compiler.
///
class ExpressionCompiler final
{
public:

    ///
    /// @brief Compiler entry point.
    ///
    /// @param[in]  kParse     Expression parse.
    /// @param[in]  kBindings  Map of variable identifiers to elements.
    /// @param[in]  kEvalType  Expression evaluation type.
    /// @param[out] kAsm       On success, points to compiled expression.
    /// @param[out] kErr       On error, if non-null, contains error info.
    ///
    /// @retval SUCCESS          Successfully compiled expression.
    /// @retval E_EXC_NULL       kParse is null.
    /// @retval E_EXC_NUM        Expression contains an invalid constant.
    /// @retval E_EXC_OVFL       A constant in the expression is too large.
    /// @retval E_EXC_ARITY      Stats function has the wrong argument count.
    /// @retval E_EXC_WIN        Stats function window was too large.
    /// @retval E_EXC_FUNC       Unknown function in expression.
    /// @retval E_EXC_ELEM       Unknown variable in expression.
    /// @retval E_EXC_ELEM_NULL  kBindings contains a null value.
    ///
    static Result compile(const Ref<const ExpressionParse> kParse,
                          const Map<String, IElement*> kBindings,
                          const ElementType kEvalType,
                          Ref<const ExpressionAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ExpressionCompiler() = delete;

private:

    ///
    /// @brief Converts a constant token to F64.
    ///
    /// @param[in]  kTok  Token to convert.
    /// @param[out] kRet  On success, contains converted value.
    /// @param[out] kErr  On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionCompiler::compile().
    ///
    static Result tokenToF64(const Token& kTok,
                             F64& kRet,
                             ErrorInfo* const kErr);

    ///
    /// @brief Compiles a stats function call.
    ///
    /// @param[in]       kParse     Parse tree rooted at function call.
    /// @param[in]       kBindings  Element symbol table.
    /// @param[out]      kNode      On success, contains compiled function.
    /// @param[in, out]  kWs        Compilation workspace.
    /// @param[out]      kErr       On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionCompiler::compile().
    ///
    static Result compileStatsFunc(const Ref<const ExpressionParse> kParse,
                                   const Map<String, IElement*>& kBindings,
                                   Ref<IExprNode<F64>>& kNode,
                                   ExpressionAssembly::Workspace& kWs,
                                   ErrorInfo* const kErr);

    ///
    /// @brief Compiles a function call.
    ///
    /// @param[in]       kParse     Parse tree rooted at function call.
    /// @param[in]       kBindings  Element symbol table.
    /// @param[out]      kNode      On success, contains compiled function.
    /// @param[in, out]  kWs        Compilation workspace.
    /// @param[out]      kErr       On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionCompiler::compile().
    ///
    static Result compileFunction(const Ref<const ExpressionParse> kParse,
                                  const Map<String, IElement*>& kBindings,
                                  Ref<IExprNode<F64>>& kNode,
                                  ExpressionAssembly::Workspace& kWs,
                                  ErrorInfo* const kErr);

    ///
    /// @brief Compiles an operator and its operands.
    ///
    /// @param[in]       kParse     Parse tree rooted at operator.
    /// @param[in]       kBindings  Element symbol table.
    /// @param[out]      kNode      On success, contains compiled operator.
    /// @param[in, out]  kWs        Compilation workspace.
    /// @param[out]      kErr       On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionCompiler::compile().
    ///
    static Result compileOperator(const Ref<const ExpressionParse> kParse,
                                  const Map<String, IElement*>& kBindings,
                                  Ref<IExprNode<F64>>& kNode,
                                  ExpressionAssembly::Workspace& kWs,
                                  ErrorInfo* const kErr);

    ///
    /// @brief Main compilation method.
    ///
    /// @param[in]       kParse     Expression parse.
    /// @param[in]       kBindings  Element symbol table.
    /// @param[out]      kNode      On success, contains compiled expression.
    /// @param[in, out]  kWs        Compilation workspace.
    /// @param[out]      kErr       On error, if non-null, contains error info.
    ///
    /// @returns See ExpressionCompiler::compile().
    ///
    static Result compileImpl(const Ref<const ExpressionParse> kParse,
                              const Map<String, IElement*>& kBindings,
                              Ref<IExprNode<F64>>& kNode,
                              ExpressionAssembly::Workspace& kWs,
                              ErrorInfo* const kErr);
};

} // namespace Sf

#endif
