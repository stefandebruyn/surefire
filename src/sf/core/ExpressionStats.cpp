#include "sf/core/ExpressionStats.hpp"

RollAvgNode::RollAvgNode(const IExpressionStats& kStats) : mStats(kStats)
{
}

F64 RollAvgNode::evaluate() const
{
    // TODO: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    return const_cast<IExpressionStats&>(mStats).mean();
}
