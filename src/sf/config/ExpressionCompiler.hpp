#ifndef SF_EXPRESSION_COMPILER_HPP
#define SF_EXPRESSION_COMPILER_HPP

#include "sf/config/ExpressionParser.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/StateVector.hpp"

namespace ExpressionCompiler
{
    class Assembly final
    {
    public:

        Assembly(const IExpression* const kRoot,
                 const std::vector<const IExpression*> kNodes);

        ~Assembly();

        const IExpression* root() const;

    private:

        const IExpression* const mRoot;

        const std::vector<const IExpression*> mNodes;
    };

    Result compile(const std::shared_ptr<ExpressionParser::Parse> kParse,
                   const bool kArithmetic,
                   const StateVector& kSv,
                   std::shared_ptr<ExpressionCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);
}

#endif
