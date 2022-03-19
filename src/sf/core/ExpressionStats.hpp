#ifndef SF_EXPRESSION_STATS_HPP
#define SF_EXPRESSION_STATS_HPP

#include "sf/core/Expression.hpp"

class IExpressionStats
{
public:

    IExpressionStats() = default;

    virtual ~IExpressionStats() = default;

    virtual void update() = 0;

    virtual F64 mean() = 0;

    virtual F64 median() = 0;

    virtual F64 min() = 0;

    virtual F64 max() = 0;

    virtual F64 range() = 0;

    IExpressionStats(const IExpressionStats&) = delete;
    IExpressionStats(IExpressionStats&&) = delete;
    IExpressionStats& operator=(const IExpressionStats&) = delete;
    IExpressionStats& operator=(IExpressionStats&&) = delete;
};

template<typename T>
class ExpressionStats final : public IExpressionStats
{
public:

    ExpressionStats(IExprNode<T>& kExpr,
                    T* const kArrA,
                    T* const kArrB,
                    const U32 kCapacity) :
        mExpr(kExpr),
        mHist(kArrA),
        mSorted(kArrB),
        mCapacity(kCapacity),
        mUpdates(0),
        mSize(0),
        mSum()
    {
    }

    void update() final override
    {
        if ((mCapacity == 0) || (mHist == nullptr))
        {
            return;
        }

        // Evaluate expression.
        const T val = mExpr.evaluate();

        // Insert value into ring buffer and save the old value.
        const U32 insertIdx = (mUpdates++ % mCapacity);
        const T oldVal = mHist[insertIdx];
        mHist[insertIdx] = val;

        // Update size.
        mSize = ((mUpdates < mCapacity) ? mUpdates : mCapacity);

        // Add value to rolling sum.
        mSum += val;

        // If an old value was just overwritten, subtract it from the rolling
        // sum.
        if (mUpdates > mCapacity)
        {
            mSum -= oldVal;
        }
    }

    F64 mean() final override
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        return (static_cast<F64>(mSum) / mSize);
    }

    F64 median() final override
    {
        if ((mSize == 0) || (mHist == nullptr) || (mSorted == nullptr))
        {
            return 0.0;
        }

        // Copy history into sorted array.
        for (U32 i = 0; i < mSize; ++i)
        {
            mSorted[i] = mHist[i];
        }

        // Insertion sort the history copy.
        for (U32 i = 0; i < mSize; ++i)
        {
            for (U32 j = 0; j < i; ++j)
            {
                if (mSorted[i] < mSorted[j])
                {
                    const T tmp = mSorted[j];
                    mSorted[j] = mSorted[i];
                    mSorted[i] = tmp;
                }
            }
        }

        // If history size is even, return average of middle two values.
        if ((mSize % 2) == 0)
        {
            const F64 a = static_cast<F64>(mSorted[(mSize / 2) - 1]);
            const F64 b = static_cast<F64>(mSorted[mSize / 2]);
            return (a + ((b - a) / 2.0));
        }

        // History size is odd, so return the middle element.
        return static_cast<F64>(mSorted[mSize / 2]);
    }

    F64 min() final override
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        T minVal = mHist[0];
        for (U32 i = 1; i < mSize; ++i)
        {
            if (mHist[i] < minVal)
            {
                minVal = mHist[i];
            }
        }

        return static_cast<F64>(minVal);
    }

    F64 max() final override
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        T maxVal = mHist[0];
        for (U32 i = 1; i < mSize; ++i)
        {
            if (mHist[i] > maxVal)
            {
                maxVal = mHist[i];
            }
        }

        return static_cast<F64>(maxVal);
    }

    F64 range() final override
    {
        return (max() - min());
    }

private:

    T* const mHist;

    T* const mSorted;

    const U32 mCapacity;

    IExprNode<T>& mExpr;

    U32 mUpdates;

    U32 mSize;

    T mSum;
};

class RollAvgNode final : public IExprNode<F64>
{
public:

    RollAvgNode(IExpressionStats& kStats);

    F64 evaluate() final override;

private:

    IExpressionStats& mStats;
};


#endif
