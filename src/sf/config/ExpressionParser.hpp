#ifndef SF_EXPRESSION_PARSER_HPP
#define SF_EXPRESSION_PARSER_HPP

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/config/Tokenizer.hpp"

namespace ExpressionParser
{
    struct Parse final
    {
        Token data;
        Ref<ExpressionParser::Parse> left;
        Ref<ExpressionParser::Parse> right;
        bool func;
    };

    Result parse(TokenIterator kIt,
                 Ref<const ExpressionParser::Parse>& kParse,
                 ErrorInfo* const kErr);
}

#endif
