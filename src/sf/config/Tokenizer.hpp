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
/// @file  sf/config/Tokenizer.hpp
/// @brief Config file tokenizer.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_TOKENIZER_HPP
#define SF_TOKENIZER_HPP

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/Token.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

///
/// @brief Config file tokenizer.
///
/// @see Token
///
namespace Tokenizer
{
    ///
    /// @brief Tokenizer entry point, taking a path to the file to tokenize.
    ///
    /// @param[in]  kFilePath  Path to file to tokenize.
    /// @param[in]  kToks      On success, contains tokens.
    /// @param[out] kErr       On error, if non-null, contains error info.
    ///
    /// @retval SUCCESS        Successfully tokenized input.
    /// @retval E_TOK_INVALID  Input contains an invalid token.
    /// @retval E_TOK_FILE     Failed to open input file.
    ///
    Result tokenize(String kFilePath, Vec<Token>& kToks, ErrorInfo* const kErr);

    ///
    /// @brief Tokenizer entry point, taking an input stream to tokenize.
    ///
    /// @see Tokenizer::tokenize(String, ...)
    ///
    Result tokenize(std::istream& kIs,
                    Vec<Token>& kToks,
                    ErrorInfo* const kErr);
}

#endif
