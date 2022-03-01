#ifndef SFA_EXPRESSION_PARSER_HPP
#define SFA_EXPRESSION_PARSER_HPP

#include <memory>

#include "sfa/sup/TokenIterator.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"
#include "sfa/sup/ConfigErrorInfo.hpp"

namespace ExpressionParser
{
    struct Parse final
    {
        Token data;
        std::shared_ptr<Parse> left;
        std::shared_ptr<Parse> right;
    };

    Result parse(TokenIterator& kIt,
                 std::shared_ptr<Parse>& kParse,
                 ConfigErrorInfo* kConfigErr);
}

#endif
