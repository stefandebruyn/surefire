#ifndef SF_EXPRESSION_PARSER_HPP
#define SF_EXPRESSION_PARSER_HPP

#include <memory>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/config/Tokenizer.hpp"

namespace ExpressionParser
{
    struct Parse final
    {
        Token data;
        std::shared_ptr<Parse> left;
        std::shared_ptr<Parse> right;
        bool func;
    };

    Result parse(TokenIterator kIt,
                 std::shared_ptr<Parse>& kParse,
                 ErrorInfo* const kErr);
}

#endif
