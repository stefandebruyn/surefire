#ifndef SFA_EXPRESSION_HPP
#define SFA_EXPRESSION_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/core/Element.hpp"

template<typename T>
class IExpr
{
public:

    virtual T evaluate() const = 0;
};

template<typename T>
class ConstExpr final : public IExpr<T>
{
public:

    ConstExpr(const T kVal) : mVal(kVal)
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
class ElementExpr final : public IExpr<T>
{
public:

    ElementExpr(Element<T>& kElem) : mElem(kElem)
    {
    }

    T evaluate() const final override
    {
        return mElem.read();
    }

private:

    Element<T>& mElem;
};

template<typename T, typename TLhs = T, typename TRhs = TLhs>
class BinOpExpr final : public IExpr<T>
{
public:

    typedef T (*Operator)(const TLhs kLhs, const TRhs kRhs);

    BinOpExpr(const Operator kOp,
              const IExpr<TLhs>& kLeft,
              const IExpr<TRhs>& kRight) :
        mOp(kOp), mLhs(kLeft), mRhs(kRight)
    {
    }

    T evaluate() const final override
    {
        return mOp(mLhs.evaluate(), mRhs.evaluate());
    }

private:

    const Operator mOp;

    const IExpr<TLhs>& mLhs;

    const IExpr<TRhs>& mRhs;
};

template<typename T, typename TRhs = T>
class UnaryOpExpr final : public IExpr<T>
{
public:

    typedef T (*Operator)(const TRhs kRhs);

    UnaryOpExpr(const Operator kOp, const IExpr<TRhs>& kRhs) :
        mOp(kOp), mRhs(kRhs)
    {
    }

    T evaluate() const final override
    {
        return mOp(mRhs.evaluate());
    }

private:

    const Operator mOp;

    const IExpr<TRhs>& mRhs;
};

/////////////////////////////// Binary Operators ///////////////////////////////

template<typename T, typename TLhs, typename TRhs>
T add(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs + kRhs);
}

template<typename T, typename TLhs, typename TRhs>
T subtract(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs - kRhs);
}

template<typename T, typename TLhs, typename TRhs>
T multiply(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs * kRhs);
}

template<typename T, typename TLhs, typename TRhs>
T divide(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs / kRhs);
}

template<typename TLhs, typename TRhs = TLhs>
bool equals(const TLhs kLhs, const TRhs kRhs)
{
    return (kLhs == kRhs);
}

/////////////////////////////// Unary Operators ////////////////////////////////

template<typename T, typename TRhs = T>
T negate(const TRhs kRhs)
{
    return -kRhs;
}

template<typename T, typename TRhs = T>
T bang(const TRhs kRhs)
{
    return !kRhs;
}

#endif
