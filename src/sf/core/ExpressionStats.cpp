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
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/ExpressionStats.hpp"

namespace Sf
{

IExprStatsNode::IExprStatsNode(IExpressionStats& kStats) : mStats(kStats)
{
}

const IExpressionStats& IExprStatsNode::stats() const
{
    return mStats;
}

RollAvgNode::RollAvgNode(IExpressionStats& kStats) : IExprStatsNode(kStats)
{
}

F64 RollAvgNode::evaluate()
{
    return mStats.mean();
}

IExpression::NodeType RollAvgNode::nodeType() const
{
    return IExpression::ROLL_AVG;
}

RollMedianNode::RollMedianNode(IExpressionStats& kStats) : IExprStatsNode(kStats)
{
}

F64 RollMedianNode::evaluate()
{
    return mStats.median();
}

IExpression::NodeType RollMedianNode::nodeType() const
{
    return IExpression::ROLL_MEDIAN;
}

RollMinNode::RollMinNode(IExpressionStats& kStats) : IExprStatsNode(kStats)
{
}

F64 RollMinNode::evaluate()
{
    return mStats.min();
}

IExpression::NodeType RollMinNode::nodeType() const
{
    return IExpression::ROLL_MIN;
}

RollMaxNode::RollMaxNode(IExpressionStats& kStats) : IExprStatsNode(kStats)
{
}

F64 RollMaxNode::evaluate()
{
    return mStats.max();
}

IExpression::NodeType RollMaxNode::nodeType() const
{
    return IExpression::ROLL_MAX;
}

RollRangeNode::RollRangeNode(IExpressionStats& kStats) : IExprStatsNode(kStats)
{
}

F64 RollRangeNode::evaluate()
{
    return mStats.range();
}

IExpression::NodeType RollRangeNode::nodeType() const
{
    return IExpression::ROLL_RANGE;
}

} // namespace Sf
