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
#include "sfa/config/ConfigErrorInfo.hpp"
#include "sfa/util/EnumHash.hpp"

struct Token final
{
    enum Type : U32
    {
        SECTION,
        LABEL,
        IDENTIFIER,
        OPERATOR,
        CONSTANT,
        COLON,
        NEWLINE,
        LPAREN,
        RPAREN,
        ANNOTATION,
        COMMENT
    };

    static const std::unordered_map<Type, std::string, EnumHash> names;

    static const std::map<Type, std::regex> regexes;

    Type type;
    std::string str;
    I32 lineNum;
    I32 colNum;

    bool operator==(const Token& other) const;

    bool operator!=(const Token& other) const;
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
