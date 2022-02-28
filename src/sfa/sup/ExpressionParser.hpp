#ifndef SFA_EXPRESSION_PARSER_HPP
#define SFA_EXPRESSION_PARSER_HPP

#include "sfa/sup/TokenIterator.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"
#include "sfa/sup/ConfigErrorInfo.hpp"

namespace ExpressionParser
{
    struct Parse final
    {
        Token data;
        Parse* left;
        Parse* right;
    };

    Result parse(TokenIterator& kIt,
                 Parse& kParse,
                 ConfigErrorInfo* kConfigErr);
}

#endif
