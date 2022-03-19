#include <cctype>
#include <sstream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/pal/Console.hpp"

ErrorInfo::ErrorInfo() : lineNum(-1), colNum(-1)
{
}

String ErrorInfo::prettifyError() const
{
    if (text.size() == 0)
    {
        return "`ErrorInfo::text` unset";
    }

    if (subtext.size() == 0)
    {
        return "`ErrorInfo::subtext` unset";
    }

    if ((lineNum - 1) >= static_cast<I32>(lines.size()))
    {
        return "`ErrorInfo::lineNum` out of range";
    }

    if ((lineNum >= 0) && (colNum >= 0))
    {
        std::stringstream ss;
        ss << Console::red << text << Console::reset << " @ " << filePath << ":"
           << lineNum << ":" << colNum << ":\n" << Console::cyan << "  | "
           << Console::reset << lines[lineNum - 1] << "\n" << Console::cyan
           << "  | ";

        U32 i = 0;
        for (; i < static_cast<U32>(colNum - 1); ++i)
        {
            ss << " ";
        }

        for (; (i < lines[lineNum - 1].size())
               && std::isspace(lines[lineNum - 1][i]); ++i)
        {
            ss << " ";
        }

        ss << "^ " << subtext << Console::reset;

        return ss.str();
    }

    if (filePath.size() != 0)
    {
        return (Console::red + text + Console::reset + " @ " + filePath + ": "
                + subtext);
    }

    return (Console::red + text + Console::reset + ": " + subtext);
}
