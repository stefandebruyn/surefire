#include <fstream>
#include <cctype>

#include "sfa/config/ConfigTokenizer.hpp"

const std::unordered_map<TokenType, std::string, EnumHash> gTokenNames =
{
    {TOK_NONE, "none"},
    {TOK_SECTION, "section"},
    {TOK_LABEL, "label"},
    {TOK_IDENTIFIER, "identifier"},
    {TOK_OPERATOR, "operator"},
    {TOK_CONSTANT, "constant"},
    {TOK_COLON, "colon"},
    {TOK_NEWLINE, "newline"},
    {TOK_LPAREN, "left parenthese"},
    {TOK_RPAREN, "right parenthese"},
    {TOK_ANNOTATION, "annotation"},
    {TOK_COMMENT, "comment"}
};

bool Token::operator==(const Token& other) const
{
    return ((this->type == other.type)
            && (this->str == other.str)
            && (this->lineNum == other.lineNum)
            && (this->colNum == other.colNum));
}

bool Token::operator!=(const Token& other) const
{
    return !(*this == other);
}

std::map<TokenType, std::regex> ConfigTokenizer::mTokenRegexes =
{
    {TOK_SECTION, std::regex("\\s*\\[([a-zA-Z0-9_/]+)\\]\\s*")},
    {TOK_LABEL, std::regex("\\s*([a-zA-Z0-9_]+):\\s*")},
    {TOK_CONSTANT, std::regex("\\s*(true|false|[0-9]*\\.?[0-9]+)\\s*")},
    {TOK_IDENTIFIER, std::regex("\\s*([a-zA-Z][a-zA-Z0-9_]*)\\s*")},
    {TOK_OPERATOR, std::regex("\\s*(==|!=|=|<=|<|>=|>|->|OR|AND|\\+|\\-|\\*|/)\\s*")},
    {TOK_COLON, std::regex("\\s*(:)\\s*")},
    {TOK_LPAREN, std::regex("\\s*(\\()\\s*")},
    {TOK_RPAREN, std::regex("\\s*(\\))\\s*")},
    {TOK_ANNOTATION, std::regex("\\s*(@[a-zA-Z][a-zA-Z0-9_]*)\\s*")},
    {TOK_COMMENT, std::regex("\\s*(#.*)\\s*")}
};

Result ConfigTokenizer::tokenize(std::string kFilePath,
                                 std::vector<Token>& kToks,
                                 ConfigErrorInfo* kConfigErr)
{
    std::ifstream ifs(kFilePath);
    if (ifs.is_open() == false)
    {
        if (kConfigErr != nullptr)
        {
            kConfigErr->msg = "failed to open file: " + kFilePath;
        }
        return E_FILE;
    }

    if (kConfigErr != nullptr)
    {
        kConfigErr->filePath = kFilePath;
    }

    return ConfigTokenizer::tokenize(ifs, kToks, kConfigErr);
}

Result ConfigTokenizer::tokenize(std::istream& kIs,
                                 std::vector<Token>& kToks,
                                 ConfigErrorInfo* kConfigErr)
{
    if ((kConfigErr != nullptr) && (kConfigErr->filePath.size() == 0))
    {
        kConfigErr->filePath = "(no file)";
    }

    std::string line;
    U32 lineNum = 1;
    while (std::getline(kIs, line))
    {
        // Tokenize the line.
        Result res = ConfigTokenizer::tokenizeLine(
            line, lineNum, kToks, kConfigErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // If the line was terminated by a newline, then add a newline token to
        // the token stream so that parsers can use them as delimiters.
        if ((kIs.eof() == false) && (kIs.fail() == false))
        {
            const Token newlineTok =
            {
                TOK_NEWLINE,
                "(newline)",
                static_cast<I32>(lineNum),
                static_cast<I32>(line.size() - 1)
            };
            kToks.push_back(newlineTok);
        }

        ++lineNum;
    }

    return SUCCESS;
}

Result ConfigTokenizer::tokenizeLine(const std::string& kLine,
                                     const U32 kLineNum,
                                     std::vector<Token>& kToks,
                                     ConfigErrorInfo* kConfigErr)
{
    // Index at which we'll try to match a token in the line. This index will
    // be bumped along as we parse tokens.
    U32 idx = 0;

    if (kConfigErr != nullptr)
    {
        kConfigErr->lines.push_back(kLine);
    }

    while (idx < kLine.size())
    {
        bool matched = false;

        // Loop across all token types and try to match them at the current
        // index. The order of this iteration is significant- see
        // `mTokenRegexes`.
        for (const std::pair<TokenType, std::regex>& tokType : mTokenRegexes)
        {
            // Try to match token at the current position.
            std::string substr = kLine.substr(idx);
            std::smatch match;
            if (std::regex_search(
                substr,
                match,
                tokType.second,
                std::regex_constants::match_continuous) == true)
            {
                // Compute the index of the first non-whitespace character in
                // the matched string so we can attach a column number to the
                // token.
                U32 nonWsIdx = 0;
                for (; nonWsIdx < match[0].str().size(); ++nonWsIdx)
                {
                    if (!std::isspace(match[0].str()[nonWsIdx]))
                    {
                        break;
                    }
                }

                // Match successful- if not a comment, pack into a `Token` and
                // append to the return vector.
                if (tokType.first != TOK_COMMENT)
                {
                    const Token tok =
                    {
                        tokType.first,
                        match[1].str(),
                        static_cast<I32>(kLineNum),
                        static_cast<I32>(idx + nonWsIdx + 1)
                    };
                    kToks.push_back(tok);
                }

                // Bump the line index.
                idx += match[0].str().size();
                matched = true;
                break;
            }
        }

        if (matched == false)
        {
            // Failed to match a token at the current index, so the input is
            // invalid.
            if (kConfigErr != nullptr)
            {
                kConfigErr->lineNum = kConfigErr->lines.size();
                kConfigErr->colNum = (idx + 1);
                kConfigErr->msg = "invalid token";
            }
            return E_TOKENIZE;
        }
    }

    // If we get this far, the entire line was valid.
    return SUCCESS;
}
