#include "sf/core/ExpressionStats.hpp"

RollAvgNode::RollAvgNode(IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollAvgNode::evaluate()
{
    return mStats.mean();
}
