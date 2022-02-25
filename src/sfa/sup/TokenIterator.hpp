#ifndef SFA_TOKEN_STREAM_HPP
#define SFA_TOKEN_STREAM_HPP

#include <unordered_set>

#include "sfa/sup/ConfigTokenizer.hpp"

class TokenIterator final
{
public:

    typedef std::vector<Token>::const_iterator Iterator;

    static const Token NONE;

    TokenIterator(Iterator kStart, Iterator kEnd);

    U32 idx() const;

    void seek(const U32 kIdx);

    bool eof() const;

    const Token& tok() const;

    const Token& take();

    Token::Type type() const;

    const std::string& str() const;

    void eat();

    U32 next(const std::unordered_set<Token::Type> kTypes) const;

    TokenIterator slice(const U32 kLower, const U32 kUpper);

    const Token& operator[](const U32 kIdx) const;

    U32 size() const;

private:

    Iterator mStart;

    const Iterator mEnd;

    Iterator mPos;

    const U32 mSize;
};

#endif
