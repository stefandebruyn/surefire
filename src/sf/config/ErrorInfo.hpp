#ifndef SF_CONFIG_SOURCE_HPP
#define SF_CONFIG_SOURCE_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/config/Token.hpp"
#include "sf/core/BasicTypes.hpp"

struct ErrorInfo final
{
    static void set(ErrorInfo* const kErr,
                    const Token& kTokErr,
                    const String kText,
                    const String kSubtext);

    String text;

    String filePath;

    I32 lineNum;

    I32 colNum;

    String subtext;

    Vec<String> lines;

    ErrorInfo();

    String prettifyError() const;
};

#endif
