#ifndef SF_EXPRESSION_PARSE_HPP
#define SF_EXPRESSION_PARSE_HPP

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/config/Tokenizer.hpp"

class ExpressionParse final
{
public:

    Token data;

    Ref<const ExpressionParse> left;

    Ref<const ExpressionParse> right;

    bool func;

    static Result parse(TokenIterator kIt,
                        Ref<const ExpressionParse>& kParse,
                        ErrorInfo* const kErr);

private:

    struct MutNode final
    {
        Token data;
        Ref<ExpressionParse::MutNode> left;
        Ref<ExpressionParse::MutNode> right;
        bool func;
    };

    static Result popSubexpression(
        std::stack<Token>& kStack,
        std::stack<Ref<ExpressionParse::MutNode>>& kNodes,
        ErrorInfo* const kErr);

    static Result parseFunctionCall(TokenIterator kIt,
                                    Ref<ExpressionParse::MutNode>& kNode,
                                    ErrorInfo* const kErr);

    static void expandDoubleIneq(const Ref<ExpressionParse::MutNode> kNode);

    static Result parseImpl(TokenIterator& kIt,
                            Ref<ExpressionParse::MutNode>& kParse,
                            ErrorInfo* const kErr);

    static void convertTree(Ref<ExpressionParse::MutNode> kFrom,
                            Ref<const ExpressionParse>& kTo);

    ExpressionParse(const Token kData,
                    const Ref<const ExpressionParse> kLeft,
                    const Ref<const ExpressionParse> kRight,
                    const bool kFunc);
};

#endif
