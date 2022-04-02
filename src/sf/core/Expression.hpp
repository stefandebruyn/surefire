////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/Expression.hpp
/// @brief Objects for representing expression trees.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_EXPRESSION_HPP
#define SF_EXPRESSION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Result.hpp"

///
/// @brief Abstract interface for an expression tree.
///
class IExpression
{
public:

    ///
    /// @brief Destructor.
    ///
    virtual ~IExpression() = default;

    ///
    /// @brief Gets the type which the expression evaluates to.
    ///
    /// @return Expression evaluation type.
    ///
    virtual ElementType type() const = 0;
};

///
/// @brief Abstract interface for an expression tree node which evaluates to
/// a particular type.
///
/// @tparam T  Node evaluation type.
///
template<typename T>
class IExprNode : public IExpression
{
public:

    ///
    /// @brief Constructor.
    ///
    IExprNode() = default;

    ///
    /// @brief Destructor.
    ///
    virtual ~IExprNode() = default;

    ///
    /// @brief Evaluates the expression tree rooted at this node.
    ///
    /// @return  Expression value.
    ///
    virtual T evaluate() = 0;

    ///
    /// @see IExpression::type()
    ///
    ElementType type() const final override;

    IExprNode(IExprNode<T>&) = delete;
    IExprNode(IExprNode<T>&&) = delete;
    IExprNode<T>& operator=(IExprNode<T>&) = delete;
    IExprNode<T>& operator=(IExprNode<T>&&) = delete;
};

///
/// @brief Expression tree leaf node that evaluates to a constant.
///
/// @tparam T  Node evaluation type.
///
template<typename T>
class ConstExprNode final : public IExprNode<T>
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kVal  Node value.
    ///
    ConstExprNode(const T kVal) : mVal(kVal)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mVal;
    }

private:

    ///
    /// @brief Node value.
    ///
    const T mVal;
};

///
/// @brief Expression tree leaf node that evaluates to a state vector element.
///
/// @tparam T  Element type.
///
template<typename T>
class ElementExprNode final : public IExprNode<T>
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kElem  Element which node evaluates to. The element must live
    ///                   at least as long as the expression node.
    ///
    ElementExprNode(const Element<T>& kElem) : mElem(kElem)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mElem.read();
    }

private:

    ///
    /// @brief Element which node evaluates to.
    ///
    const Element<T>& mElem;
};

///
/// @brief Expression tree node that performs a binary operation on two
/// subtrees.
///
/// @tparam T         Evaluation type.
/// @tparam TOperand  Operand type.
///
template<typename T, typename TOperand = T>
class BinOpExprNode final : public IExprNode<T>
{
public:

    ///
    /// @brief Signature for a binary operation.
    ///
    /// @param[in] kLhs  LHS operand.
    /// @param[in] kRhs  RHS operand.
    ///
    /// @return Operation result.
    ///
    typedef T (*Operator)(const TOperand kLhs, const TOperand kRhs);

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kOp   Node operation.
    /// @param[in] kLhs  Left operand subtree.
    /// @param[in] kRhs  Right operand subtree.
    ///
    BinOpExprNode(const Operator kOp,
                  IExprNode<TOperand>& kLhs,
                  IExprNode<TOperand>& kRhs) :
        mOp(kOp), mLhs(kLhs), mRhs(kRhs)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mOp(mLhs.evaluate(), mRhs.evaluate());
    }

private:

    ///
    /// @brief Node operation.
    ///
    const Operator mOp;

    ///
    /// @brief Left operand subtree.
    ///
    IExprNode<TOperand>& mLhs;

    ///
    /// @brief Right operand subtree.
    ///
    IExprNode<TOperand>& mRhs;
};

///
/// @brief Expression tree node that performs a unary operation on a subtree.
///
/// @note The implementation refers to the operand as the RHS since most unary
/// operators appear to the left of their operand, but this is not a
/// requirement.
///
/// @tparam T         Evaluation type.
/// @tparam TOperand  Operand type.
///
template<typename T, typename TOperand = T>
class UnaryOpExprNode final : public IExprNode<T>
{
public:

    ///
    /// @brief Signature for a unary operation.
    ///
    /// @param[in] kRhs  Operand.
    ///
    /// @return Operation result.
    ///
    typedef T (*Operator)(const TOperand kRhs);

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kOp     Node operation.
    /// @param[in] kRight  Operand subtree.
    ///
    UnaryOpExprNode(const Operator kOp, IExprNode<TOperand>& kRhs) :
        mOp(kOp), mRhs(kRhs)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mOp(mRhs.evaluate());
    }

private:

    ///
    /// @brief Node operation.
    ///
    const Operator mOp;

    ///
    /// @brief Operand subtree.
    ///
    IExprNode<TOperand>& mRhs;
};

///
/// @brief Namespace of templates for getting numeric type limits, similar to
/// the <limits> STL header.
///
namespace Limits
{
    ///
    /// @brief Gets the minimum value representable by a numeric type. For
    /// floating types, this is negative infinity.
    ///
    /// @tparam T  Type.
    ///
    /// @return Minimum value representable by type.
    ///
    template<typename T>
    T min();

    ///
    /// @brief Gets the maximum value representable by a numeric type. For
    /// floating types, this is positive infinity.
    ///
    /// @tparam T  Type.
    ///
    /// @return Maximum value representable by type.
    ///
    template<typename T>
    T max();
}

///
/// @brief A "safe" cast operation that saturates at numeric limits when
/// precision loss prevents a precise cast and converts NaNs to 0 or false.
/// Casting an integral type to a floating type when the integer cannot be
/// accurately represented as a float has the same effect as a static cast
/// (i.e., the integer is approximated as a float).
///
/// @remark This is currently only defined to/from F64 since these are the only
/// casts required by ExpressionCompiler.
///
/// @tparam T         Cast (destination) type.
/// @tparam TOperand  Operand (source) type.
///
/// @param[in] kRhs  Value to cast.
///
/// @return Cast value.
///
template<typename T, typename TOperand>
T safeCast(const TOperand kRhs);

#endif
