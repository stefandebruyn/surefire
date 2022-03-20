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

    struct Node final
    {
        Token data;
        Ref<Node> left;
        Ref<Node> right;
        bool func;
    };

    static Result popSubexpression(
        std::stack<Token>& kStack,
        std::stack<Ref<ExpressionParse::Node>>& kNodes,
        ErrorInfo* const kErr);

    static Result parseFunctionCall(TokenIterator kIt,
                                    Ref<ExpressionParse::Node>& kNode,
                                    ErrorInfo* const kErr);

    static void expandDoubleIneq(const Ref<ExpressionParse::Node> kNode);

    static Result parseImpl(TokenIterator& kIt,
                            Ref<ExpressionParse::Node>& kParse,
                            ErrorInfo* const kErr);

    static void convertTree(Ref<ExpressionParse::Node> kFrom,
                            Ref<const ExpressionParse>& kTo);

    ExpressionParse(const Token kData,
                    const Ref<const ExpressionParse> kLeft,
                    const Ref<const ExpressionParse> kRight,
                    const bool kFunc);
};

#endif
