#include <algorithm>

#include "sf/config/ConfigUtil.hpp"

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
