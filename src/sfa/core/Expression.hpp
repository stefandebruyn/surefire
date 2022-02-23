#ifndef SFA_EXPRESSION_HPP
#define SFA_EXPRESSION_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"

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

template<typename TOp, typename T, typename TLeft = T, typename TRight = TLeft>
class BinOpExpr final : public IExpr<T>
{
public:

    BinOpExpr(const IExpr<TLeft>& kLeft, const IExpr<TRight>& kRight) :
        mOp(), mLeft(kLeft), mRight(kRight)
    {
    }

    T evaluate() const final override
    {
        return mOp(mLeft.evaluate(), mRight.evaluate());
    }

private:

    const TOp mOp;

    const IExpr<TLeft>& mLeft;

    const IExpr<TRight>& mRight;
};

template<typename TOp, typename T, typename TRight = T>
class UnaryOpExpr final : public IExpr<T>
{
public:

    UnaryOpExpr(const IExpr<TRight>& kRight) : mOp(), mRight(kRight)
    {
    }

    T evaluate() const final override
    {
        return mOp(mRight.evaluate());
    }

private:

    const TOp mOp;

    const IExpr<TRight>& mRight;
};

#endif
