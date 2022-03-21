#include <algorithm>

#include "sf/config/ConfigUtil.hpp"

const Set<String> ConfigUtil::reserved =
{
    "STATE_VECTOR",
    "LOCAL",
    "IF",
    "ELSE",
    "ENTRY",
    "STEP",
    "EXIT",
    "T",
    "G",
    "S"
};

void ConfigUtil::setError(ErrorInfo* const kErr,
                          const Token& kTokErr,
                          const String kText,
                          const String kSubtext)
{
    if (kErr != nullptr)
    {
        kErr->lineNum = kTokErr.lineNum;
        kErr->colNum = kTokErr.colNum;
        kErr->text = kText;
        kErr->subtext = kSubtext;
    }
}

bool ConfigUtil::checkEof(const TokenIterator& kIt,
                          const Token& kTokLast,
                          const String kErrText,
                          ErrorInfo* const kErr)
{
    if (kIt.eof())
    {
        if (kErr != nullptr)
        {
            kErr->lineNum = kTokLast.lineNum;
            kErr->colNum = (kTokLast.colNum + kTokLast.str.size());
            kErr->text = kErrText;
            kErr->subtext = "unexpected end of file";
        }
        return true;
    }

    return false;
}
