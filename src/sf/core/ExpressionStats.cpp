////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/ExpressionStats.hpp"

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
