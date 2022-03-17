#ifndef SF_CONFIG_TOKENIZER_HPP
#define SF_CONFIG_TOKENIZER_HPP

#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "sf/config/EnumHash.hpp"
#include "sf/config/ErrorInfo.hpp"
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
        COMMA = 13
    };

    static const std::unordered_map<Type, std::string, EnumHash> names;

    static const std::vector<std::pair<Type, std::regex>> regexes;

    Type type;
    std::string str;
    I32 lineNum;
    I32 colNum;

    bool operator==(const Token& kOther) const;

    bool operator!=(const Token& kOther) const;

    friend std::ostream& operator<<(std::ostream& kOs, const Token& kTok);
};

namespace Tokenizer
{
    Result tokenize(std::string kFilePath,
                    std::vector<Token>& kToks,
                    ErrorInfo* const kErr);

    Result tokenize(std::istream& kIs,
                    std::vector<Token>& kToks,
                    ErrorInfo* const kErr);
}

#endif
