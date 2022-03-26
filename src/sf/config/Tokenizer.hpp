#ifndef SF_TOKENIZER_HPP
#define SF_TOKENIZER_HPP

#include "sf/config/EnumHash.hpp"
#include "sf/config/ErrorInfo.hpp"
#include "sf/config/Token.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

namespace Tokenizer
{
    Result tokenize(String kFilePath, Vec<Token>& kToks, ErrorInfo* const kErr);

    Result tokenize(std::istream& kIs,
                    Vec<Token>& kToks,
                    ErrorInfo* const kErr);
}

#endif
