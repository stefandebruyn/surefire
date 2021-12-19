#ifndef SFA_CONFIG_TOKENIZER_HPP
#define SFA_CONFIG_TOKENIZER_HPP

#include <string>
#include <map>
#include <regex>
#include <vector>
#include <istream>

#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

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

struct Token final
{
    TokenType type;
    U32 which;
    std::string str;

    bool operator==(const Token& other) const;

    bool operator!=(const Token& other) const;
};

struct ConfigErrorInfo
{
    std::string fileName;
    std::string line;
    I32 lineNum;
    I32 colNum;
    std::string msg;

    std::string prettify() const;
};

class Tokenizer final
{
public:

    Tokenizer() = delete;

    static Result tokenize(std::string kFilePath,
                           std::vector<Token>& kRet,
                           ConfigErrorInfo* kErrInfo);

    static Result tokenize(std::istream& kIs,
                           std::vector<Token>& kRet,
                           ConfigErrorInfo* kErrInfo);

private:

    static std::map<TokenType, std::regex> mTokenRegexes;

    static Result tokenizeLine(const std::string& kLine,
                               std::vector<Token>& kRet,
                               ConfigErrorInfo* kErrInfo);
};

#endif