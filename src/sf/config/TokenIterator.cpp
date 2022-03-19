#include <algorithm>

#include "sf/config/TokenIterator.hpp"

const Token TokenIterator::NONE = {Token::NONE, "(none)", -1, -1};

TokenIterator::TokenIterator(Iterator kStart, Iterator kEnd) :
    mStart(kStart), mEnd(kEnd), mPos(kStart), mSize(kEnd - kStart)
{
    if (mStart > mEnd)
    {
        mStart = mEnd;
        mPos = mStart;
    }
}

U32 TokenIterator::idx() const
{
    return (mPos - mStart);
}

void TokenIterator::seek(const U32 kIdx)
{
    if (kIdx <= mSize)
    {
        mPos = (mStart + kIdx);
    }
    else
    {
        mPos = mEnd;
    }
}

bool TokenIterator::eof() const
{
    return (mPos == mEnd);
}

const Token& TokenIterator::tok() const
{
    if (this->eof())
    {
        return NONE;
    }
    return *mPos;
}

const Token& TokenIterator::take()
{
    const Token& ret = this->tok();
    if (!this->eof())
    {
        ++mPos;
    }
    this->eat();
    return ret;
}

Token::Type TokenIterator::type() const
{
    return this->tok().type;
}

const String& TokenIterator::str() const
{
    return this->tok().str;
}

void TokenIterator::eat()
{
    while (!this->eof() && (this->type() == Token::NEWLINE))
    {
        ++mPos;
    }
}

U32 TokenIterator::next(const Set<Token::Type> kTypes) const
{
    Iterator i = mPos;
    while (i != mEnd)
    {
        const Token& tok = *i;
        if (kTypes.find(tok.type) != kTypes.end())
        {
            break;
        }
        ++i;
    }
    return (i - mStart);
}

TokenIterator TokenIterator::slice(const U32 kLower, const U32 kUpper)
{
    return TokenIterator((mStart + kLower), (mStart + kUpper));
}

const Token& TokenIterator::operator[](const U32 kIdx) const
{
    if (kIdx >= mSize)
    {
        return NONE;
    }
    return *(mStart + kIdx);
}

U32 TokenIterator::size() const
{
    return mSize;
}
