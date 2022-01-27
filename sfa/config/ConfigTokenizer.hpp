#ifndef SFA_CONFIG_TOKENIZER_HPP
#define SFA_CONFIG_TOKENIZER_HPP

#include <string>
#include <regex>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"
#include "sfa/config/ConfigErrorInfo.hpp"
#include "sfa/util/EnumHash.hpp"

struct Token final
{
    enum Type : U32
    {
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

class ConfigTokenizer final
{
public:

    static Result tokenize(std::string kFilePath,
                           std::vector<Token>& kToks,
                           ConfigErrorInfo* kConfigErr);

    static Result tokenize(std::istream& kIs,
                           std::vector<Token>& kToks,
                           ConfigErrorInfo* kConfigErr);

    ConfigTokenizer() = delete;

private:

    static Result tokenizeLine(const std::string& kLine,
                               const U32 kLineNum,
                               std::vector<Token>& kToks,
                               ConfigErrorInfo* kConfigErr);
};

#endif
