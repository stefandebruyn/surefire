#ifndef SF_EXPRESSION_COMPILER_HPP
#define SF_EXPRESSION_COMPILER_HPP

#include "sf/config/ExpressionParse.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/ExpressionStats.hpp"
#include "sf/core/StateVector.hpp"

class ExpressionAssembly final
{
public:

    static Result compile(const Ref<const ExpressionParse> kParse,
                          const Vec<Ref<const StateVector>> kSvs,
                          const ElementType kEvalType,
                          Ref<const ExpressionAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ~ExpressionAssembly();

    Ref<IExpression> root() const;

    Vec<Ref<IExpressionStats>> stats() const;

private:

    struct Workspace
    {
        Vec<Ref<IExpression>> exprNodes;
        Vec<Ref<IExpressionStats>> exprStats;
        Vec<const char*> statArrs;
    };

    static Result tokenToF64(const Token& kTok,
                             F64& kRet,
                             ErrorInfo* const kErr);

    static Result compileExprStatsFunc(
        const Ref<const ExpressionParse> kParse,
        const Vec<Ref<const StateVector>> kSvs,
        Ref<IExprNode<F64>>& kNode,
        ExpressionAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result compileFunction(const Ref<const ExpressionParse> kParse,
                                  const Vec<Ref<const StateVector>> kSvs,
                                  Ref<IExprNode<F64>>& kNode,
                                  ExpressionAssembly::Workspace& kWs,
                                  ErrorInfo* const kErr);

    static Result compileOperator(const Ref<const ExpressionParse> kParse,
                                  const Vec<Ref<const StateVector>> kSvs,
                                  Ref<IExprNode<F64>>& kNode,
                                  ExpressionAssembly::Workspace& kWs,
                                  ErrorInfo* const kErr);

    static Result compileImpl(const Ref<const ExpressionParse> kParse,
                              const Vec<Ref<const StateVector>> kSvs,
                              Ref<IExprNode<F64>>& kNode,
                              ExpressionAssembly::Workspace& kWs,
                              ErrorInfo* const kErr);

    ExpressionAssembly(const Ref<IExpression> kRoot,
                       const Vec<Ref<IExpression>>& kNodes,
                       const Vec<Ref<IExpressionStats>>& kStats,
                       const Vec<const char*>& kStatArrs);

    const Ref<IExpression> mRoot;

    const Vec<Ref<IExpression>> mNodes;

    const Vec<Ref<IExpressionStats>> mStats;

    const Vec<const char*> mStatArrs;
};

#endif
