#include "sf/core/ExpressionStats.hpp"

RollAvgNode::RollAvgNode(IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollAvgNode::evaluate()
{
    return mStats.mean();
}

RollMedianNode::RollMedianNode(IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollMedianNode::evaluate()
{
    return mStats.median();
}

RollMinNode::RollMinNode(IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollMinNode::evaluate()
{
    return mStats.min();
}

RollMaxNode::RollMaxNode(IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollMaxNode::evaluate()
{
    return mStats.max();
}

RollRangeNode::RollRangeNode(IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollRangeNode::evaluate()
{
    return mStats.range();
}
