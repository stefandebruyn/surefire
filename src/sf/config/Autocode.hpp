#ifndef SF_AUTOCODE_HPP
#define SF_AUTOCODE_HPP

#include <iomanip>
#include <limits>
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

        mOs << Autocode::formatStep(kFmt, kArgs...) << "\n";
    }

    void operator()();

    void flush();

    template<typename T, typename... TArgs>
    static String format(String kFmt, T kT, TArgs... kArgs)
    {
        return Autocode::formatStep(kFmt, kT, kArgs...);
    }

    Autocode(const Autocode&) = delete;
    Autocode(Autocode&&) = delete;
    Autocode& operator=(const Autocode&) = delete;
    Autocode& operator=(Autocode&&) = delete;

private:

    std::ostream& mOs;

    U32 mIndentLvl;

    template<typename T, typename... TArgs>
    static String& formatStep(String& kFmt, T kT, TArgs... kArgs)
    {
        // Find index of next format specifier. If not found, stop recursion.
        std::size_t pos = kFmt.find("%%");
        if (pos == std::string::npos)
        {
            return kFmt;
        }

        // Convert current value to string.
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<F64>::digits10) << kT;

        // Replace format specifier with stringified value and recurse.
        kFmt.replace(pos, 2, ss.str());
        return Autocode::formatStep(kFmt, kArgs...);
    }

    static String& formatStep(String& kStr);
};

#endif
