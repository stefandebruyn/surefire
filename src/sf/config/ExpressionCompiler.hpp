#ifndef SF_EXPRESSION_COMPILER_HPP
#define SF_EXPRESSION_COMPILER_HPP

#include <vector>

#include "sf/config/ExpressionParser.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/ExpressionStats.hpp"
#include "sf/core/StateVector.hpp"

namespace ExpressionCompiler
{
    class Assembly final
    {
    public:

        Assembly(const IExpression* const kRoot,
                 const std::vector<const IExpression*> kNodes,
                 const std::vector<IExpressionStats*> kStats);

        ~Assembly();

        const IExpression* root() const;

        const std::vector<IExpressionStats*>& stats() const;

    private:

        const IExpression* const mRoot;

        const std::vector<const IExpression*> mNodes;

        const std::vector<IExpressionStats*> mStats;
    };

    Result compile(const std::shared_ptr<ExpressionParser::Parse> kParse,
                   const std::vector<const StateVector*> kSvs,
                   const ElementType kEvalType,
                   std::shared_ptr<ExpressionCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);
}

#endif
