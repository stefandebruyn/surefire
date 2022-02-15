#include <fstream>
#include <cctype>

#include "sfa/sup/ConfigTokenizer.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace ConfigTokenizer
{
    Result tokenizeLine(const std::string& kLine,
                        const U32 kLineNum,
                        std::vector<Token>& kToks,
                        ConfigErrorInfo* kConfigErr);
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
        // `Token::regexes`.
        for (const std::pair<Token::Type, std::regex>& tokType : Token::regexes)
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
                if (tokType.first != Token::COMMENT)
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
                kConfigErr->text = "error";
                kConfigErr->subtext = "invalid token";
            }
            return E_TOK_INVALID;
        }
    }

    // If we get this far, the entire line was valid.
    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

const std::unordered_map<Token::Type, std::string, EnumHash> Token::names =
{
    {Token::SECTION, "section"},
    {Token::LABEL, "label"},
    {Token::IDENTIFIER, "identifier"},
    {Token::OPERATOR, "operator"},
    {Token::CONSTANT, "constant"},
    {Token::COLON, "colon"},
    {Token::NEWLINE, "newline"},
    {Token::LPAREN, "left parenthese"},
    {Token::RPAREN, "right parenthese"},
    {Token::ANNOTATION, "annotation"},
    {Token::COMMENT, "comment"},
    {Token::LBRACE, "left brace"},
    {Token::RBRACE, "right brace"}
};

const std::vector<std::pair<Token::Type, std::regex>> Token::regexes =
{
    {Token::SECTION, std::regex("\\s*(\\[[a-zA-Z0-9_/]+\\])\\s*")},
    {Token::LABEL, std::regex("\\s*([.][a-zA-Z][a-zA-Z0-9_\\[\\]-]+)\\s*")},
    {Token::CONSTANT, std::regex("\\s*(true|false|[0-9]*\\.?[0-9]+)\\s*")},
    {Token::OPERATOR, std::regex(
        "\\s*(==|!=|=|<=|<|>=|>|->|and|or|\\+|\\-|\\*|/)\\s*")},
    {Token::IDENTIFIER, std::regex("\\s*([a-zA-Z][a-zA-Z0-9_]*)\\s*")},
    {Token::COLON, std::regex("\\s*(:)\\s*")},
    {Token::LPAREN, std::regex("\\s*(\\()\\s*")},
    {Token::RPAREN, std::regex("\\s*(\\))\\s*")},
    {Token::ANNOTATION, std::regex("\\s*(@[a-zA-Z][a-zA-Z0-9_]*)\\s*")},
    {Token::COMMENT, std::regex("\\s*(#.*)\\s*")},
    {Token::LBRACE, std::regex("\\s*(\\{)\\s*")},
    {Token::RBRACE, std::regex("\\s*(\\})\\s*")}
};

bool Token::operator==(const Token& kOther) const
{
    return ((this->type == kOther.type)
            && (this->str == kOther.str)
            && (this->lineNum == kOther.lineNum)
            && (this->colNum == kOther.colNum));
}

bool Token::operator!=(const Token& kOther) const
{
    return !(*this == kOther);
}

std::ostream& operator<<(std::ostream& kOs, const Token& kTok)
{
    return (kOs << "Token(type=" << kTok.type << ", str=\"" << kTok.str
                << "\", lineNum=" << kTok.lineNum << ", colNum=" << kTok.colNum
                << ")");
}

Result ConfigTokenizer::tokenize(std::string kFilePath,
                                 std::vector<Token>& kToks,
                                 ConfigErrorInfo* kConfigErr)
{
    std::ifstream ifs(kFilePath);
    if (ifs.is_open() == false)
    {
        if (kConfigErr != nullptr)
        {
            kConfigErr->text = "error";
            kConfigErr->subtext = "failed to open file: " + kFilePath;
        }
        return E_FILE;
    }

    if (kConfigErr != nullptr)
    {
        kConfigErr->filePath = kFilePath;
    }

    return tokenize(ifs, kToks, kConfigErr);
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
        Result res = tokenizeLine(line, lineNum, kToks, kConfigErr);
        if (res != SUCCESS)
        {
            // Error occurred- clear return vector.
            kToks.clear();
            return res;
        }

        // If the line was terminated by a newline, then add a newline token to
        // the token stream so that parsers can use them as delimiters.
        if ((kIs.eof() == false) && (kIs.fail() == false))
        {
            const Token newlineTok =
            {
                Token::NEWLINE,
                "(newline)",
                static_cast<I32>(lineNum),
                static_cast<I32>(line.size() + 1)
            };
            kToks.push_back(newlineTok);
        }

        ++lineNum;
    }

    return SUCCESS;
}
