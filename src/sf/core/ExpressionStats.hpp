#ifndef SF_EXPRESSION_STATS_HPP
#define SF_EXPRESSION_STATS_HPP

#include "sf/core/Expression.hpp"

///
/// @brief Abstract interface for making the `ExpressionStats` template
/// polymorphic.
///
class IExpressionStats
{
public:

    ///
    /// @brief Evaluates the expression, adds the new value to the history, and
    /// discards the oldest value if the history is full.
    ///
    virtual void update() = 0;
};

///
/// @brief Computes statistics on a history of values of an expression.
///
/// @note Computations guard against overflow by saturating integer types at
/// their minimum and maximum representable values.
///
/// @note When computing stats on floating point expressions, a NaN value will
/// poison the computations and produce more NaNs. It is the responsibility of
/// upstream code to guard against NaNs.
///
/// @tparam T          Type which the expression evaluates to.
/// @tparam THistSize  Size of the history. The object will store the most
///                    recent `THistSize` values of the expression.
///
template<typename T, U32 THistSize>
class ExpressionStats final : public IExpressionStats
{
public:

    ///
    /// @brief Constructor. The value history is initially empty and all stats
    /// are zero.
    ///
    /// @param[in] kExpr  Expression to calculate stats for.
    ///
    ExpressionStats(const IExprNode<T>& kExpr) :
        mExpr(kExpr), mUpdates(0), mSize(0), mSum()
    {
    }

    ///
    /// @brief Evaluates the expression, adds the new value to the history, and
    /// discards the oldest value if the history is full.
    ///
    /// @remark This method is O(1).
    ///
    void update() final override
    {
        // Evaluate expression.
        const T val = mExpr.evaluate();

        // Insert value into ring buffer and save the old value.
        const U32 insertIdx = (mUpdates++ % THistSize);
        const T oldVal = mHist[insertIdx];
        mHist[insertIdx] = val;

        // Update size.
        mSize = ((mUpdates < THistSize) ? mUpdates : THistSize);

        // Add value to rolling sum.
        mSum = add<T>(mSum, val);

        // If an old value was just overwritten, subtract it from the rolling
        // sum.
        if (mUpdates > THistSize)
        {
            mSum = subtract<T>(mSum, oldVal);
        }
    }

    ///
    /// @brief Returns the mean of the value history. The mean of an empty
    /// history is zero.
    ///
    /// @remarks This method is O(1).
    ///
    /// @return History mean.
    ///
    F64 mean() const
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        return ((F64) mSum / mSize);
    }

    ///
    /// @brief Returns the median of the value history. The median of an empty
    /// history is zero.
    ///
    /// @remark The median is determined by insertion sorting the history,
    /// making this method O(N^2) in the worst case. This should be fast enough
    /// for even relatively large history sizes in the 100s or 1000s. A more
    /// time-efficient solution might use a heap, but the core library assumes
    /// no heap memory is available, so implementing a dynamic container like a
    /// heap in this context is tricky.
    ///
    /// @return History median.
    ///
    F64 median()
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        // Copy history into sorted array.
        for (U32 i = 0; i < mSize; ++i)
        {
            mSorted[i] = mHist[i];
        }

        // Insertion sort in ascending order the copy.
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

    ///
    /// @brief Gets the minimum value in the history. The min of an empty
    /// history is zero.
    ///
    /// @remark This method is O(N).
    ///
    /// @return History min.
    ///
    F64 min() const
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        T minVal = Limits::max<T>();
        for (U32 i = 0; i < mSize; ++i)
        {
            if (mHist[i] < minVal)
            {
                minVal = mHist[i];
            }
        }

        return static_cast<F64>(minVal);
    }

    ///
    /// @brief Gets the maximum value in the history. The max of an empty
    /// history is zero.
    ///
    /// @remark This method is O(N).
    ///
    /// @return History max.
    ///
    F64 max() const
    {
        if (mSize == 0)
        {
            return 0.0;
        }

        T maxVal = Limits::min<T>();
        for (U32 i = 0; i < mSize; ++i)
        {
            if (mHist[i] > maxVal)
            {
                maxVal = mHist[i];
            }
        }

        return static_cast<F64>(maxVal);
    }

    ///
    /// @brief Gets the range of the history. The range of an empty history is
    /// zero.
    ///
    /// @remark This method is O(N).
    ///
    /// @return History range.
    ///
    F64 range() const
    {
        return (max() - min());
    }

private:

    ///
    /// @brief Expression value history.
    ///
    T mHist[THistSize];

    ///
    /// @brief Array for storing sorted history when computing certain stats.
    ///
    T mSorted[THistSize];

    ///
    /// @brief Number of times a new value has been added to the history.
    ///
    U32 mUpdates;

    ///
    /// @brief Number of values in the history. This is between 0 and history
    /// size, inclusive.
    ///
    U32 mSize;

    ///
    /// @brief Rolling sum of values in history.
    ///
    T mSum;

    ///
    /// @brief Expression.
    ///
    const IExprNode<T>& mExpr;
};

#endif
