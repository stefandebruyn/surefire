#include <sstream>
#include <cctype>

#include "sfa/config/ConfigErrorInfo.hpp"

ConfigErrorInfo::ConfigErrorInfo() : lineNum(-1), colNum(-1)
{
}

std::string ConfigErrorInfo::prettifyError() const
{
    if (msg.size() == 0)
    {
        return "`ConfigErrorInfo::msg` unset";
    }

    if ((lineNum - 1) >= static_cast<I32>(lines.size()))
    {
        return "`ConfigErrorInfo::lineNum` out of range";
    }

    if ((lineNum >= 0) && (colNum >= 0))
    {
        std::stringstream ss;
        ss << "CONFIG ERROR @ " << filePath << ":" << lineNum
           << ":" << colNum << ":\n"
           << "  | " << lines[lineNum - 1] << "\n"
           << "  | ";

        I32 i = 0;
        for (; i < (colNum - 1); ++i)
        {
            ss << " ";
        }

        for (; (i < lines[lineNum - 1].size())
               && std::isspace(lines[lineNum - 1][i]); ++i)
        {
            ss << " ";
        }

        ss << "^ " << msg << "\n";

        return ss.str();
    }

    if (filePath.size() != 0)
    {
        return ("CONFIG ERROR @ " + filePath + ": " + msg);
    }

    return ("CONFIG ERROR: " + msg);
}
