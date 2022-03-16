#ifndef SFA_EXPRESSION_HPP
#define SFA_EXPRESSION_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/core/Element.hpp"

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

template<typename T, typename TLhs = T, typename TRhs = TLhs>
class BinOpExprNode final : public IExprNode<T>
{
public:

    typedef T (*Operator)(const TLhs kLhs, const TRhs kRhs);

    BinOpExprNode(const Operator kOp,
                  const IExprNode<TLhs>& kLeft,
                  const IExprNode<TRhs>& kRight) :
        mOp(kOp), mLhs(kLeft), mRhs(kRight)
    {
    }

    T evaluate() const final override
    {
        return mOp(mLhs.evaluate(), mRhs.evaluate());
    }

private:

    const Operator mOp;

    const IExprNode<TLhs>& mLhs;

    const IExprNode<TRhs>& mRhs;
};

template<typename T, typename TRhs = T>
class UnaryOpExprNode final : public IExprNode<T>
{
public:

    typedef T (*Operator)(const TRhs kRhs);

    UnaryOpExprNode(const Operator kOp, const IExprNode<TRhs>& kRhs) :
        mOp(kOp), mRhs(kRhs)
    {
    }

    T evaluate() const final override
    {
        return mOp(mRhs.evaluate());
    }

private:

    const Operator mOp;

    const IExprNode<TRhs>& mRhs;
};

/////////////////////////////// Binary Operators ///////////////////////////////

template<typename T, typename TLhs = T, typename TRhs  = TLhs>
T add(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs + kRhs);
}

template<typename T, typename TLhs = T, typename TRhs  = TLhs>
T subtract(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs - kRhs);
}

template<typename T, typename TLhs = T, typename TRhs  = TLhs>
T multiply(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs * kRhs);
}

template<typename T, typename TLhs = T, typename TRhs  = TLhs>
T divide(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs / kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool lessThan(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs < kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool lessThanEquals(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs <= kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool greaterThan(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs > kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool greaterThanEquals(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs >= kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool equals(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs == kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool notEquals(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs != kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool logicalAnd(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs && kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool logicalOr(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs || kRhs);
}

/////////////////////////////// Unary Operators ////////////////////////////////

template<typename T, typename TRhs = T>
T bang(const TRhs kRhs)
{
    return !kRhs;
}

template<typename T, typename TRhs>
T cast(const TRhs kRhs)
{
    return static_cast<T>(kRhs);
}

#endif
