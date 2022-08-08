////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/config/TokenIterator.hpp
/// @brief Token iterator utility for parsing token sequences.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_TOKEN_STREAM_HPP
#define SF_TOKEN_STREAM_HPP

#include "sf/config/Tokenizer.hpp"

///
/// @brief Token iterator utility for parsing token sequences.
///
class TokenIterator final
{
public:

    ///
    /// @brief Shorthand for an STL token vector iterator.
    ///
    typedef Vec<Token>::const_iterator Iterator;

    ///
    /// @brief Special token that indicates end of token stream.
    ///
    static const Token NONE;

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kStart  STL iterator at start of token stream.
    /// @param[in] kEnd    STL iterator at end of token stream.
    ///
    TokenIterator(Iterator kStart, Iterator kEnd);

    ///
    /// @brief Gets iterator index in the token stream. The first token is at
    /// index 0.
    ///
    /// @returns Iterator inudex.
    ///
    U32 idx() const;

    ///
    /// @brief Moves the iterator to an index in the token stream. The index
    /// will be ceilinged at the token stream length.
    ///
    /// @param[in] kIdx  Target index.
    ///
    void seek(const U32 kIdx);

    ///
    /// @brief Gets whether the iterator has reached the end of the token
    /// stream.
    ///
    /// @returns If end of stream.
    ///
    bool eof() const;

    ///
    /// @brief Gets the token at the current index without moving the index.
    /// If the iterator is at end of stream, Token::NONE is returned.
    ///
    /// @returns Token at current index.
    ///
    const Token& tok() const;

    ///
    /// @brief Gets the token at the current index and advances the index to the
    /// next non-newline token. If the iterator is at end of stream, Token::NONE
    /// is returned.
    ///
    /// @returns Token at current index.
    ///
    const Token& take();

    ///
    /// @brief Gets the type of the token at the current index without moving
    /// the index.
    ///
    /// @returns Type of token at current index.
    ///
    Token::Type type() const;

    ///
    /// @brief Gets the string of the token at the current index without
    /// moving the index.
    ///
    /// @returns String of token at current index.
    ///
    const String& str() const;

    ///
    /// @brief Advances the iterator index until a non-newline token or end of
    /// stream is reached.
    ///
    void eat();

    ///
    /// @brief Advances the iterator index until a token of a certain type or
    /// end of stream is reached. If the iterator is already at a token with one
    /// of the specified types, the index does not move.
    ///
    /// @param[in] kTypes  Token types to stop on.
    ///
    /// @returns  Number of indices advanced.
    ///
    U32 next(const Set<Token::Type> kTypes) const;

    ///
    /// @brief Gets an iterator covering a slice of this iterator. The lower
    /// index becomes index 0 in the new iterator. If the lower index is >= the
    /// upper index, the new iterator covers an empty token sequence.
    ///
    /// @param[in] kLower  Lower index of slice, inclusive.
    /// @param[in] kUpper  Upper index of slice, exclusive.
    ///
    /// @returns Slice iterator.
    ///
    TokenIterator slice(const U32 kLower, const U32 kUpper);

    ///
    /// @brief Gets the token at an index in the token sequence without moving
    /// the index. If the index is out of bounds, Token::NONE is returned.
    ///
    /// @returns kIdx  Index.
    ///
    /// @returns Token at index.
    ///
    const Token& operator[](const U32 kIdx) const;

    ///
    /// @brief Gets the number of tokens in the token sequence.
    ///
    /// @returns Token sequence size.
    ///
    U32 size() const;

private:

    ///
    /// @brief STL iterator at start of token sequence.
    ///
    Iterator mStart;

    ///
    /// @brief STL iterator at end of token sequence.
    ///
    const Iterator mEnd;

    ///
    /// @brief STL iterator at current index.
    ///
    Iterator mPos;

    ///
    /// @brief Token sequence size.
    ///
    const U32 mSize;
};

#endif
