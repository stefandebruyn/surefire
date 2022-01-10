#include <fstream>

#include "sfa/ConfigTokenizer.hpp"

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
            && (this->which == other.which)
            && (this->str == other.str));
}

bool Token::operator!=(const Token& other) const
{
    return !(*this == other);
}

U32 Tokenizer::mLineNum = 0;

std::map<TokenType, std::regex> Tokenizer::mTokenRegexes =
{
    {TOK_SECTION, std::regex("\\s*\\[([a-zA-Z0-9_/]+)\\]")},
    {TOK_LABEL, std::regex("\\s*([a-zA-Z0-9_]+):")},
    {TOK_CONSTANT, std::regex("\\s*(true|false|[0-9]*.?[0-9]+)")},
    {TOK_IDENTIFIER, std::regex("\\s*([a-zA-Z][a-zA-Z0-9_]*)")},
    {TOK_OPERATOR, std::regex("\\s*(==|!=|=|<=|<|>=|>|->|OR|AND|\\+|\\-|\\*|/)")},
    {TOK_COLON, std::regex("\\s*(:)")},
    {TOK_LPAREN, std::regex("\\s*(\\()")},
    {TOK_RPAREN, std::regex("\\s*(\\))")},
    {TOK_ANNOTATION, std::regex("\\s*(@[a-zA-Z][a-zA-Z0-9_]*)")},
    {TOK_COMMENT, std::regex("\\s*(#.*)")}
};

Result Tokenizer::tokenize(std::string kFilePath,
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

    return Tokenizer::tokenize(ifs, kToks, kConfigErr);
}

Result Tokenizer::tokenize(std::istream& kIs,
                           std::vector<Token>& kToks,
                           ConfigErrorInfo* kConfigErr)
{
    mLineNum = 0;

    if ((kConfigErr != nullptr) && (kConfigErr->filePath.size() == 0))
    {
        kConfigErr->filePath = "(no file)";
    }

    std::string line;
    while (std::getline(kIs, line))
    {
        // Tokenize the line.
        Result res = Tokenizer::tokenizeLine(line, kToks, kConfigErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // If the `getline` was terminated by a newline, then add a
        // `TOK_NEWLINE` to the token stream so that parsers can use them as
        // delimiters if they want.
        if ((kIs.eof() == false) && (kIs.fail() == false))
        {
            const Token newlineTok =
            {
                TOK_NEWLINE,
                0,
                "(newline)",
                static_cast<I32>(mLineNum),
                static_cast<I32>(line.size() - 1)
            };
            kToks.push_back(newlineTok);
        }

        ++mLineNum;
    }

    return SUCCESS;
}

Result Tokenizer::tokenizeLine(const std::string& kLine,
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
                // Match successful- if not a comment, pack into a `Token`.
                if (tokType.first != TOK_COMMENT)
                {
                    const Token tok =
                    {
                        tokType.first,
                        0,
                        match[1].str(),
                        static_cast<I32>(mLineNum),
                        static_cast<I32>(idx)
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
                kConfigErr->lineNum = (kConfigErr->lines.size() - 1);
                kConfigErr->colNum = idx;
                kConfigErr->msg = "invalid token";
            }
            return E_TOKENIZE;
        }
    }

    // If we get this far, the entire line was valid.
    return SUCCESS;
}
