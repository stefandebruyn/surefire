#ifndef SFA_CONFIG_TOKENIZER_HPP
#define SFA_CONFIG_TOKENIZER_HPP

#include <string>
#include <regex>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/sup/ConfigErrorInfo.hpp"
#include "sfa/sup/EnumHash.hpp"

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
        RBRACE = 12
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

namespace ConfigTokenizer
{
    Result tokenize(std::string kFilePath,
                    std::vector<Token>& kToks,
                    ConfigErrorInfo* kConfigErr);

    Result tokenize(std::istream& kIs,
                    std::vector<Token>& kToks,
                    ConfigErrorInfo* kConfigErr);
}

#endif
