#ifndef SFA_CONFIG_TOKENIZER_HPP
#define SFA_CONFIG_TOKENIZER_HPP

#include <string>
#include <map>
#include <regex>
#include <vector>
#include <istream>
#include <unordered_map>

#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"
#include "sfa/ConfigInfo.hpp"
#include "sfa/EnumHash.hpp"

enum TokenType : U32
{
    TOK_NONE = 0,
    TOK_SECTION = 1,
    TOK_LABEL = 2,
    TOK_IDENTIFIER = 3,
    TOK_OPERATOR = 4,
    TOK_CONSTANT = 5,
    TOK_COLON = 6,
    TOK_NEWLINE = 7,
    TOK_LPAREN = 8,
    TOK_RPAREN = 9
};

extern const std::unordered_map<TokenType, std::string, EnumHash> gTokenNames;

struct Token final
{
    TokenType type;
    U32 which;
    std::string str;
    I32 lineNum;
    I32 colNum;

    bool operator==(const Token& other) const;

    bool operator!=(const Token& other) const;
};

class Tokenizer final
{
public:

    Tokenizer() = delete;

    static Result tokenize(std::string kFilePath,
                           std::vector<Token>& kRet,
                           ConfigInfo* kConfigInfo);

    static Result tokenize(std::istream& kIs,
                           std::vector<Token>& kRet,
                           ConfigInfo* kConfigInfo);

private:

    static U32 mLineNum;

    static std::map<TokenType, std::regex> mTokenRegexes;

    static Result tokenizeLine(const std::string& kLine,
                               std::vector<Token>& kRet,
                               ConfigInfo* kConfigInfo);
};

#endif
