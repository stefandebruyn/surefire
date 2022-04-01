#ifndef SF_EXPRESSION_COMPILER_HPP
#define SF_EXPRESSION_COMPILER_HPP

#include "sf/config/ExpressionParser.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/ExpressionStats.hpp"
#include "sf/core/StateVector.hpp"

class ExpressionAssembly final
{
public:

    Ref<IExpression> root() const;

    Vec<Ref<IExpressionStats>> stats() const;

private:

    friend class ExpressionCompiler;

    struct Workspace
    {
        Vec<Ref<IExpression>> exprNodes;
        Vec<Ref<IExpressionStats>> exprStats;
        Vec<Ref<Vec<U8>>> statArrs;
        Ref<IExpression> rootNode;
    };

    const ExpressionAssembly::Workspace mWs;

    ExpressionAssembly(const ExpressionAssembly::Workspace& kWs);
};

class ExpressionCompiler final
{
public:

    static Result compile(const Ref<const ExpressionParse> kParse,
                          const Map<String, IElement*> kBindings,
                          const ElementType kEvalType,
                          Ref<const ExpressionAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ExpressionCompiler() = delete;

private:

    static Result tokenToF64(const Token& kTok,
                             F64& kRet,
                             ErrorInfo* const kErr);

    static Result compileStatsFunc(const Ref<const ExpressionParse> kParse,
                                   const Map<String, IElement*>& kBindings,
                                   Ref<IExprNode<F64>>& kNode,
                                   ExpressionAssembly::Workspace& kWs,
                                   ErrorInfo* const kErr);

    static Result compileFunction(const Ref<const ExpressionParse> kParse,
                                  const Map<String, IElement*>& kBindings,
                                  Ref<IExprNode<F64>>& kNode,
                                  ExpressionAssembly::Workspace& kWs,
                                  ErrorInfo* const kErr);

    static Result compileOperator(const Ref<const ExpressionParse> kParse,
                                  const Map<String, IElement*>& kBindings,
                                  Ref<IExprNode<F64>>& kNode,
                                  ExpressionAssembly::Workspace& kWs,
                                  ErrorInfo* const kErr);

    static Result compileImpl(const Ref<const ExpressionParse> kParse,
                              const Map<String, IElement*>& kBindings,
                              Ref<IExprNode<F64>>& kNode,
                              ExpressionAssembly::Workspace& kWs,
                              ErrorInfo* const kErr);
};

#endif
