////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include <cctype>
#include <sstream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/pal/Console.hpp"

void ErrorInfo::set(ErrorInfo* const kErr,
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

ErrorInfo::ErrorInfo() : lineNum(-1), colNum(-1)
{
}

String ErrorInfo::prettifyError() const
{
    // Check that text is set.
    if (text.size() == 0)
    {
        return "`ErrorInfo::text` unset";
    }

    // Check that subtext is set.
    if (subtext.size() == 0)
    {
        return "`ErrorInfo::subtext` unset";
    }

    // Check that line number is in range.
    if ((lineNum - 1) >= static_cast<I32>(lines.size()))
    {
        return "`ErrorInfo::lineNum` out of range";
    }

    // If line numbers  are non-negative, this error implicates a specific
    // token in a file.
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

    // If the error implicates a file, include that in the message.
    if (filePath.size() != 0)
    {
        return (Console::red + text + Console::reset + " @ " + filePath + ": "
                + subtext);
    }

    // If we got this far, this is a general error that does not implicate a
    // file, so just print the text and subtext.
    return (Console::red + text + Console::reset + ": " + subtext);
}
