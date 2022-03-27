#ifndef SF_AUTOCODE_HPP
#define SF_AUTOCODE_HPP

#include <ostream>
#include <sstream>

#include "sf/core/BasicTypes.hpp"
#include "sf/config/StlTypes.hpp"

class Autocode final
{
public:

    Autocode(std::ostream& kOs);

    ~Autocode();

    void increaseIndent();

    void decreaseIndent();

    template<typename... TArgs>
    void operator()(String kFmt, TArgs... kArgs)
    {
        // Add indenting.
        for (U32 i = 0; i < mIndentLvl; ++i)
        {
            mOs << "    ";
        }

        mOs << Autocode::format(kFmt, kArgs...) << "\n";
    }

    void operator()();

    void flush();

private:

    std::ostream& mOs;

    U32 mIndentLvl;

    template<typename T, typename... TArgs>
    static String& format(String& kFmt, T kT, TArgs... kArgs)
    {
        // Find index of next format specifier. If not found, stop recursion.
        std::size_t pos = kFmt.find("%%");
        if (pos == std::string::npos)
        {
            return kFmt;
        }

        // Convert current value to string.
        std::stringstream ss;
        ss << kT;

        // Replace format specifier with stringified value and recurse.
        kFmt.replace(pos, 2, ss.str());
        return Autocode::format(kFmt, kArgs...);
    }

    static String& format(String& kStr);
};

#endif
