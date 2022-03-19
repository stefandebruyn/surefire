#ifndef SF_EXPRESSION_HPP
#define SF_EXPRESSION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Result.hpp"

class IExpression
{
public:

    virtual ~IExpression() = default;

    virtual ElementType type() const = 0;
};

template<typename T>
class IExprNode : public IExpression
{
public:

    IExprNode() = default;

    virtual ~IExprNode() = default;

    virtual T evaluate() const = 0;

    ElementType type() const final override;

    IExprNode(const IExprNode<T>&) = delete;
    IExprNode(IExprNode<T>&&) = delete;
    IExprNode<T>& operator=(const IExprNode<T>&) = delete;
    IExprNode<T>& operator=(IExprNode<T>&&) = delete;
};

template<typename T>
class ConstExprNode final : public IExprNode<T>
{
public:

    ConstExprNode(const T kVal) : mVal(kVal)
    {
    }

    T evaluate() const final override
    {
        return mVal;
    }

private:

    const T mVal;
};

template<typename T>
class ElementExprNode final : public IExprNode<T>
{
public:

    ElementExprNode(const Element<T>& kElem) : mElem(kElem)
    {
    }

    T evaluate() const final override
    {
        return mElem.read();
    }

private:

    const Element<T>& mElem;
};

template<typename T, typename TOperand = T>
class BinOpExprNode final : public IExprNode<T>
{
public:

    typedef T (*Operator)(const TOperand kLhs, const TOperand kRhs);

    BinOpExprNode(const Operator kOp,
                  const IExprNode<TOperand>& kLeft,
                  const IExprNode<TOperand>& kRight) :
        mOp(kOp), mLhs(kLeft), mRhs(kRight)
    {
    }

    T evaluate() const final override
    {
        return mOp(mLhs.evaluate(), mRhs.evaluate());
    }

private:

    const Operator mOp;

    const IExprNode<TOperand>& mLhs;

    const IExprNode<TOperand>& mRhs;
};

template<typename T, typename TOperand = T>
class UnaryOpExprNode final : public IExprNode<T>
{
public:

    typedef T (*Operator)(const TOperand kRhs);

    UnaryOpExprNode(const Operator kOp, const IExprNode<TOperand>& kRhs) :
        mOp(kOp), mRhs(kRhs)
    {
    }

    T evaluate() const final override
    {
        return mOp(mRhs.evaluate());
    }

private:

    const Operator mOp;

    const IExprNode<TOperand>& mRhs;
};

namespace Limits
{
    template<typename T>
    T min();

    template<typename T>
    T max();
}

template<typename T, typename TOperand>
T safeCast(const TOperand kRhs);

#endif