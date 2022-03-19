#ifndef SF_EXPRESSION_COMPILER_HPP
#define SF_EXPRESSION_COMPILER_HPP

#include "sf/config/ExpressionParser.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/ExpressionStats.hpp"
#include "sf/core/StateVector.hpp"

namespace ExpressionCompiler
{
    class Assembly final
    {
    public:

        Assembly(IExpression* const kRoot,
                 const Vec<const IExpression*>& kNodes,
                 const Vec<IExpressionStats*>& kStats,
                 const Vec<const char*>& kStatArrs);

        ~Assembly();

        IExpression* root() const;

        const Vec<IExpressionStats*>& stats() const;

    private:

        IExpression* const mRoot;

        const Vec<const IExpression*> mNodes;

        const Vec<IExpressionStats*> mStats;

        const Vec<const char*> mStatArrs;
    };

    Result compile(const Ref<const ExpressionParser::Parse> kParse,
                   const Vec<const StateVector*> kSvs,
                   const ElementType kEvalType,
                   Ref<const ExpressionCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);
}

#endif
