////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/Token.hpp
/// @brief Token object that represents a token in a tokenized config file.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_TOKEN_HPP
#define SF_TOKEN_HPP

#include <iostream>
#include <regex>

#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StlTypes.hpp"

///
/// @brief Token object that represents a token in a tokenized config file. Also
/// acts as a namespace for token metadata.
///
/// @remark The range of token types is designed for all framework config files.
/// Not all config file formats may use all token types.
///
struct Token final
{
    ///
    /// @brief Token types.
    ///
    enum Type : I32
    {
        NONE = -1,      ///< No token. Indicates the end of a token stream.
        SECTION = 0,    ///< A section, like "[state_vector]".
        LABEL = 1,      ///< A label, like ".entry".
        IDENTIFIER = 2, ///< An identifier, like "foo".
        OPERATOR = 3,   ///< An operator, like "+".
        CONSTANT = 4,   ///< A constant, like "true" or "1.522".
        COLON = 5,      ///< A colon ":".
        NEWLINE = 6,    ///< A newline "\n".
        LPAREN = 7,     ///< A left parenthese "(".
        RPAREN = 8,     ///< A right parenthese ")".
        ANNOTATION = 9, ///< An annotation, like "@alias".
        COMMENT = 10,   ///< A comment; these are omitted from tokenizer output.
        LBRACE = 11,    ///< A left brace "{".
        RBRACE = 12,    ///< A right brace "}".
        COMMA = 13,     ///< A comma ",".
        KEYWORD = 14    ///< A keyword, like "->".
    };

    ///
    /// @brief Map of token types to human-readable names.
    ///
    static const Map<Type, String> names;

    ///
    /// @brief Map of token types to regexes that match them.
    ///
    static const Vec<std::pair<Type, std::regex>> regexes;

    ///
    /// @brief Token type.
    ///
    Type type;

    ///
    /// @brief Token text.
    ///
    String str;

    ///
    /// @brief Line number of token.
    ///
    I32 lineNum;

    ///
    /// @brief Column number of token.
    ///
    I32 colNum;

    ///
    /// @brief If an operator token, points to operator info. This is populated
    /// during tokenization.
    ///
    const OpInfo* opInfo;

    ///
    /// @brief If a type identifier, points to type info. This is populated
    /// during tokenization.
    ///
    const TypeInfo* typeInfo;

    ///
    /// @brief Tokens are equal when their type, string, line number, and column
    /// number match.
    ///
    /// @param[in] kOther  Token to compare to.
    ///
    /// @returns True if tokens are equal, false otherwise.
    ///
    bool operator==(const Token& kOther) const;

    ///
    /// @see Token::operator==()
    ///
    bool operator!=(const Token& kOther) const;

    ///
    /// @brief Allows printing tokens to output streams.
    ///
    /// @param[in] kOs   Output stream.
    /// @param[in] kTok  Token to print.
    ///
    friend std::ostream& operator<<(std::ostream& kOs, const Token& kTok);
};

#endif
