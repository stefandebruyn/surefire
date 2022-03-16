#ifndef SFA_EXPRESSION_COMPILER_HPP
#define SFA_EXPRESSION_COMPILER_HPP

#include "sfa/core/Expression.hpp"
#include "sfa/core/StateVector.hpp"
#include "sfa/sup/ExpressionParser.hpp"

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
                   ConfigErrorInfo* const kConfigErr);
}

#endif
