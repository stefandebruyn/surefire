#include <sstream>
#include <cctype>

#include "sfa/ConfigInfo.hpp"

ConfigErrorInfo::ConfigErrorInfo() : lineNum(-1), colNum(-1)
{
}

std::string ConfigInfo::prettifyError() const
{
    if (error.lineNum >= static_cast<I32>(lines.size()))
    {
        return "`ConfigInfo::error::lineNum` out of range";
    }

    if ((error.lineNum >= 0) && (error.colNum >= 0))
    {
        std::stringstream ss;
        ss << "CONFIG ERROR @ " << filePath << ":" << (error.lineNum + 1)
           << ":" << (error.colNum + 1) << "\n"
           << "   | " << lines[error.lineNum] << "\n"
           << "   | ";

        I32 i = 0;
        for (; i < error.colNum; ++i)
        {
            ss << " ";
        }

        for (; i < lines[error.lineNum].size()
            && std::isspace(lines[error.lineNum][i]); ++i)
        {
            ss << " ";
        }

        ss << "^ " << error.msg << "\n";

        return ss.str();
    }

    return ("CONFIG ERROR: " + error.msg);
}
