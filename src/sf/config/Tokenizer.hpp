#ifndef SF_CONFIG_TOKENIZER_HPP
#define SF_CONFIG_TOKENIZER_HPP

#include <iostream>
#include <regex>

#include "sf/config/EnumHash.hpp"
#include "sf/config/ErrorInfo.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

struct Token final
{
    enum Type : I32
    {
        NONE = -1,
        SECTION = 0,
        LABEL = 1,
        IDENTIFIER = 2,
        OPERATOR = 3,
        CONSTANT = 4,
        COLON = 5,
        NEWLINE = 6,
        LPAREN = 7,
        RPAREN = 8,
        ANNOTATION = 9,
        COMMENT = 10,
        LBRACE = 11,
        RBRACE = 12,
        COMMA = 13,
        KEYWORD = 14
    };

    static const Map<Type, String, EnumHash> names;

    static const Vec<std::pair<Type, std::regex>> regexes;

    Type type;
    String str;
    I32 lineNum;
    I32 colNum;
    const OpInfo* opInfo;
    const TypeInfo* typeInfo;

    bool operator==(const Token& kOther) const;

    bool operator!=(const Token& kOther) const;

    friend std::ostream& operator<<(std::ostream& kOs, const Token& kTok);
};

namespace Tokenizer
{
    Result tokenize(String kFilePath, Vec<Token>& kToks, ErrorInfo* const kErr);

    Result tokenize(std::istream& kIs,
                    Vec<Token>& kToks,
                    ErrorInfo* const kErr);
}

#endif
