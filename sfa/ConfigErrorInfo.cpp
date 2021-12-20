#include <sstream>
#include <cctype>

#include "sfa/ConfigErrorInfo.hpp"

ConfigErrorInfo::ConfigErrorInfo() : lineNum(-1), colNum(-1)
{
}

std::string ConfigErrorInfo::prettifyError() const
{
    if (msg.size() == 0)
    {
        return "`ConfigErrorInfo::msg` unset";
    }

    if (lineNum >= static_cast<I32>(lines.size()))
    {
        return "`ConfigErrorInfo::lineNum` out of range";
    }

    if ((lineNum >= 0) && (colNum >= 0))
    {
        std::stringstream ss;
        ss << "CONFIG ERROR @ " << filePath << ":" << (lineNum + 1)
           << ":" << (colNum + 1) << "\n"
           << "   | " << lines[lineNum] << "\n"
           << "   | ";

        I32 i = 0;
        for (; i < colNum; ++i)
        {
            ss << " ";
        }

        for (; i < lines[lineNum].size()
            && std::isspace(lines[lineNum][i]); ++i)
        {
            ss << " ";
        }

        ss << "^ " << msg << "\n";

        return ss.str();
    }

    return ("CONFIG ERROR: " + msg);
}
