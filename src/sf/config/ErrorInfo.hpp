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
/// @file  sf/config/ErrorInfo.hpp
/// @brief Object for capturing info about a parser, compiler, etc. error.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ERROR_INFO_HPP
#define SF_ERROR_INFO_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/config/Token.hpp"
#include "sf/core/BasicTypes.hpp"

///
/// @brief Object for capturing info about a parser, compiler, etc. error.
///
struct ErrorInfo final
{
    ///
    /// @brief Sets the fields of an ErrorInfo.
    ///
    ///
    /// @param[in] kErr      ErrorInfo to set. If null, nothing happens.
    /// @param[in] kTokErr   Token implicated in error.
    /// @param[in] kText     Error text - the "big picture" error message, like
    ///                      "compiler error".
    /// @param[in] kSubtext  Error subtext - the specific error message, like
    ///                      "unknown variable".
    ///
    static void set(ErrorInfo* const kErr,
                    const Token& kTokErr,
                    const String kText,
                    const String kSubtext);

    ///
    /// @brief Error text.
    ///
    String text;

    ///
    /// @brief Path to file containing error.
    ///
    String filePath;

    ///
    /// @brief Error line number.
    ///
    I32 lineNum;

    ///
    /// @brief Error column number.
    ///
    I32 colNum;

    ///
    /// @brief Error subtext.
    ///
    String subtext;

    ///
    /// @brief Contents of file containing error. This will be procedurally
    /// populated as the ErrorInfo passes through the parsing phase of a
    /// compilation process.
    ///
    Vec<String> lines;

    ///
    /// @brief Constructor.
    ///
    ErrorInfo();

    ///
    /// @brief Gets a pretty string of the error that can be directly printed
    /// to the console.
    ///
    /// Three types of error messages are possible, depending on member values:
    ///
    ///   1. lineNum and colNum are non-negative - Error implicates a specific
    ///      token in a file. Assumes filePath and lines are also populated.
    ///   2. lineNum and colNum are negative, filePath is populated - Error
    ///      implicates a file in general.
    ///   3. lineNum and colNum are negative, filePath is empty - General error
    ///      that does not implicate a file.
    ///
    /// @returns Pretty error string.
    ///
    String prettifyError() const;
};

#endif
