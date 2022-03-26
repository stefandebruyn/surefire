#include "sf/config/Token.hpp"

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
