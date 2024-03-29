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
/// @file  sf/core/ExpressionStats.hpp
/// @brief ExpressionStats template for computing statistics on an expression.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_EXPRESSION_STATS_HPP
#define SF_EXPRESSION_STATS_HPP

#include "sf/core/Expression.hpp"

namespace Sf
{

///
/// @brief Abstract interface for the ExpressionStats template. An
/// ExpressionStats computes stats on a rolling window of past values. The
/// object is updated periodically by external code, and stats are queried
/// through the interface.
///
/// @remark Used to implement stat functions like roll_avg() in the state
/// machine config language.
///
class IExpressionStats
{
public:

    ///
    /// @brief Constructor.
    ///
    IExpressionStats() = default;

    ///
    /// @brief Destructor.
    ///
    virtual ~IExpressionStats() = default;

    ///
    /// @brief Re-evaluate the underlying expression and update the rolling
    /// window. If the window is full, the oldest value is discarded.
    ///
    /// @note To mirror the behavior of ExprOpFuncs::safeCast(), NaNs in the
    /// rolling window are treated like zeros.
    ///
    virtual void update() = 0;

    ///
    /// @brief Gets the mean of the rolling window. If the window is not full
    /// (i.e., update() has been called fewer times than the window size), only
    /// values in the window (and not empty spaces) are factored into the
    /// calculation. If the window is empty, 0 is returned.
    ///
    /// @return Rolling window mean.
    ///
    virtual F64 mean() = 0;

    ///
    /// @brief Gets the median of the rolling window. If the window is not full
    /// (i.e., update() has been called fewer times than the window size), only
    /// values in the window (and not empty spaces) are factored into the
    /// calculation. If the window is empty, 0 is returned.
    ///
    /// @return Rolling window median.
    ///
    virtual F64 median() = 0;

    ///
    /// @brief Gets the min of the rolling window. If the window is not full
    /// (i.e., update() has been called fewer times than the window size), only
    /// values in the window (and not empty spaces) are factored into the
    /// calculation. If the window is empty, 0 is returned.
    ///
    /// @return Rolling window min.
    ///
    virtual F64 min() = 0;

    ///
    /// @brief Gets the max of the rolling window. If the window is not full
    /// (i.e., update() has been called fewer times than the window size), only
    /// values in the window (and not empty spaces) are factored into the
    /// calculation. If the window is empty, 0 is returned.
    ///
    /// @return Rolling window max.
    ///
    virtual F64 max() = 0;

    ///
    /// @brief Gets the range of the rolling window. If the window is not full
    /// (i.e., update() has been called fewer times than the window size), only
    /// values in the window (and not empty spaces) are factored into the
    /// calculation. If the window is empty, 0 is returned.
    ///
    /// @return Rolling window range.
    ///
    virtual F64 range() = 0;

    ///
    /// @brief Gets the size of the rolling window. This is the capacity of the
    /// window and not the number of values actually occupying it.
    ///
    /// @return Rolling window size.
    ///
    virtual U32 size() const = 0;

    ///
    /// @brief Gets the expression which stats are computed on.
    ///
    /// @return Expression.
    ///
    virtual const IExpression& expr() const = 0;

    IExpressionStats(const IExpressionStats&) = delete;
    IExpressionStats(IExpressionStats&&) = delete;
    IExpressionStats& operator=(const IExpressionStats&) = delete;
    IExpressionStats& operator=(IExpressionStats&&) = delete;
};

///
/// @brief Computes stats on a rolling window of past values which an expression
/// has evaluated to.
///
/// @tparam T  Evaluation type of expression which stats are computed on.
///
/// @see IExpressionStats
///
template<typename T>
class ExpressionStats final : public IExpressionStats
{
public:

    ///
    /// @brief Constructor. The rolling window is initially empty.
    ///
    /// @remark The object requires two storage arrays. The first stores the
    /// rolling window in chronological order, and the second is used to sort
    /// the array for calculating the median.
    ///
    /// @param[in] kExpr  Expression which stats are computed on.
    /// @param[in] kArrA  Storage array for exclusive use by the object, kSize
    ///                   elements long. If this array is null, no stats will be
    ///                   computed.
    /// @param[in] kArrB  Additional storage array for exclusive use by the
    ///                   object, kSize elements long. If this array is null,
    ///                   all stats but median will be computed.
    /// @param[in] kSize  Size of the rolling window.
    ///
    ExpressionStats(IExprNode<T>& kExpr,
                    T* const kArrA,
                    T* const kArrB,
                    const U32 kSize) :
        mExpr(kExpr),
        mHist(kArrA),
        mSorted(kArrB),
        mSize(kSize),
        mUpdates(0),
        mCnt(0),
        mSum()
    {
    }

    ///
    /// @see IExpressionStats::update()
    ///
    /// @remark This method is O(1).
    ///
    void update() final override
    {
        if ((mSize == 0) || (mHist == nullptr))
        {
            return;
        }

        // Evaluate expression.
        T val = mExpr.evaluate();

        // A NaN becomes 0, the same behavior as ExprOpFuncs::safeCast().
        if (val != val)
        {
            val = 0;
        }

        // Insert value into ring buffer and save the old value.
        const U32 insertIdx = (mUpdates++ % mSize);
        const T oldVal = mHist[insertIdx];
        mHist[insertIdx] = val;

        // Update size.
        mCnt = ((mUpdates < mSize) ? mUpdates : mSize);

        // Add value to rolling sum.
        mSum += val;

        // If an old value was just overwritten, subtract it from the rolling
        // sum.
        if (mUpdates > mSize)
        {
            mSum -= oldVal;
        }
    }

    ///
    /// @see IExpressionStats::mean()
    ///
    /// @remark This method is O(1).
    ///
    F64 mean() final override
    {
        if (mCnt == 0)
        {
            return 0.0;
        }

        return (mSum / mCnt);
    }

    ///
    /// @see IExpressionStats::median()
    ///
    /// @remark This method is O(n^2) in the worst case due to the use of
    /// insertion sort. The applications targeted by the state machine config
    /// language typically require small histories of 10s or 100s of values,
    /// so this performance is acceptable.
    ///
    F64 median() final override
    {
        if ((mCnt == 0) || (mHist == nullptr) || (mSorted == nullptr))
        {
            return 0.0;
        }

        // Copy history into sorted array.
        for (U32 i = 0; i < mCnt; ++i)
        {
            mSorted[i] = mHist[i];
        }

        // Insertion sort the history copy.
        for (U32 i = 0; i < mCnt; ++i)
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
        if ((mCnt % 2) == 0)
        {
            const F64 a =
                ExprOpFuncs::safeCast<F64, T>(mSorted[(mCnt / 2) - 1]);
            const F64 b = ExprOpFuncs::safeCast<F64, T>(mSorted[mCnt / 2]);
            return (a + ((b - a) / 2.0));
        }

        // History size is odd, so return the middle element.
        return ExprOpFuncs::safeCast<F64, T>(mSorted[mCnt / 2]);
    }

    ///
    /// @see IExpressionStats::min()
    ///
    /// @remark This method is O(n).
    ///
    F64 min() final override
    {
        if (mCnt == 0)
        {
            return 0.0;
        }

        T minVal = mHist[0];
        for (U32 i = 1; i < mCnt; ++i)
        {
            if (mHist[i] < minVal)
            {
                minVal = mHist[i];
            }
        }

        return ExprOpFuncs::safeCast<F64, T>(minVal);
    }

    ///
    /// @see IExpressionStats::max()
    ///
    /// @remark This method is O(n).
    ///
    F64 max() final override
    {
        if (mCnt == 0)
        {
            return 0.0;
        }

        T maxVal = mHist[0];
        for (U32 i = 1; i < mCnt; ++i)
        {
            if (mHist[i] > maxVal)
            {
                maxVal = mHist[i];
            }
        }

        return ExprOpFuncs::safeCast<F64, T>(maxVal);
    }

    ///
    /// @see IExpressionStats::range()
    ///
    /// @remark This method is O(1).
    ///
    F64 range() final override
    {
        return (max() - min());
    }

    ///
    /// @see IExpressionStats::size()
    ///
    U32 size() const final override
    {
        return mSize;
    }

    ///
    /// @see IExpressionStats::expr()
    ///
    const IExpression& expr() const final override
    {
        return static_cast<IExpression&>(mExpr);
    }

private:

    ///
    /// @brief Expression which stats are computed on.
    ///
    IExprNode<T>& mExpr;

    ///
    /// @brief Array which stores the rolling window chronologically.
    ///
    T* const mHist;

    ///
    /// @brief Array used to sort the rolling window when computing the median.
    ///
    T* const mSorted;

    ///
    /// @brief Size of the rolling window.
    ///
    const U32 mSize;

    ///
    /// @brief Number of updates performed (calls to update()).
    ///
    U32 mUpdates;

    ///
    /// @brief Number of values in the rolling window. This is initially 0 and
    /// saturates at the window size when it fills up.
    ///
    U32 mCnt;

    ///
    /// @brief Sum of the rolling window, updated as calls to update() are made.
    ///
    F64 mSum;
};

///
/// @brief Abstract interface for an expression node which evaluates to a stat
/// computed by an IExpressionStats.
///
class IExprStatsNode : public IExprNode<F64>
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kStats  IExpressionStats used by node.
    ///
    IExprStatsNode(IExpressionStats& kStats);

    ///
    /// @see IExpression::evaluate()
    ///
    virtual F64 evaluate() = 0;

    ///
    /// @see IExpression::nodeType()
    ///
    virtual IExpression::NodeType nodeType() const = 0;

    ///
    /// @brief Gets the node's underlying IExpressionStats.
    ///
    /// @return Underlying IExpressionStats.
    ///
    const IExpressionStats& stats() const;

protected:

    ///
    /// @brief IExpressionStats used by node.
    ///
    IExpressionStats& mStats;
};

///
/// @brief Expression node which evaluates to the rolling average of an
/// expression.
///
class RollAvgNode final : public IExprStatsNode
{
public:

    ///
    /// @see IExprStatsNope::IExprStatsNode()
    ///
    RollAvgNode(IExpressionStats& kStats);

    ///
    /// @brief Gets the current rolling average computed by the underlying
    /// IExpressionStats. This method does NOT invoke
    /// IExpressionStats::update(); this is done by external code.
    ///
    /// @return Expression rolling average.
    ///
    F64 evaluate() final override;

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override;
};

///
/// @brief Expression node which evaluates to the rolling median of an
/// expression.
///
class RollMedianNode final : public IExprStatsNode
{
public:

    ///
    /// @see IExprStatsNope::IExprStatsNode()
    ///
    RollMedianNode(IExpressionStats& kStats);

    ///
    /// @brief Gets the current rolling median computed by the underlying
    /// IExpressionStats. This method does NOT invoke
    /// IExpressionStats::update(); this is done by external code.
    ///
    /// @return Expression rolling median.
    ///
    F64 evaluate() final override;

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override;
};

///
/// @brief Expression node which evaluates to the rolling min of an expression.
///
class RollMinNode final : public IExprStatsNode
{
public:

    ///
    /// @see IExprStatsNope::IExprStatsNode()
    ///
    RollMinNode(IExpressionStats& kStats);

    ///
    /// @brief Gets the current rolling min computed by the underlying
    /// IExpressionStats. This method does NOT invoke
    /// IExpressionStats::update(); this is done by external code.
    ///
    /// @return Expression rolling min.
    ///
    F64 evaluate() final override;

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override;
};

///
/// @brief Expression node which evaluates to the rolling max of an expression.
///
class RollMaxNode final : public IExprStatsNode
{
public:

    ///
    /// @see IExprStatsNope::IExprStatsNode()
    ///
    RollMaxNode(IExpressionStats& kStats);

    ///
    /// @brief Gets the current rolling max computed by the underlying
    /// IExpressionStats. This method does NOT invoke
    /// IExpressionStats::update(); this is done by external code.
    ///
    /// @return Expression rolling max.
    ///
    F64 evaluate() final override;

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override;
};

///
/// @brief Expression node which evaluates to the rolling range of an
/// expression.
///
class RollRangeNode final : public IExprStatsNode
{
public:

    ///
    /// @see IExprStatsNope::IExprStatsNode()
    ///
    RollRangeNode(IExpressionStats& kStats);

    ///
    /// @brief Gets the current rolling range computed by the underlying
    /// IExpressionStats. This method does NOT invoke
    /// IExpressionStats::update(); this is done by external code.
    ///
    /// @return Expression rolling range.
    ///
    F64 evaluate() final override;

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override;
};

} // namespace Sf

#endif
