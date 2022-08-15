////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/Autocode.hpp
/// @brief Autocoding utility object.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_AUTOCODE_HPP
#define SF_AUTOCODE_HPP

#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>

#include "sf/core/BasicTypes.hpp"
#include "sf/config/StlTypes.hpp"

///
/// @brief Autocoding utility object.
///
class Autocode final
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kOs  Output stream to write autocode to.
    ///
    Autocode(std::ostream& kOs);

    ///
    /// @brief Destructor. Flushes output stream.
    ///
    ~Autocode();

    ///
    /// @brief Increases indent level by 4 spaces.
    ///
    void increaseIndent();

    ///
    /// @brief Decreases indent level by 4 spaces.
    ///
    void decreaseIndent();

    ///
    /// @brief Writes a line of autocode.
    ///
    /// @param[in] kFmt   Format string. The format specifier is "%%",
    ///                   regardless of type.
    /// @param[in] kArgs  Format arguments. Each argument will be converted to
    ///                   a string by left-shifting it into a stream, so this
    ///                   operator must be defined for the argument type.
    ///
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

    ///
    /// @brief Writes an empty line.
    ///
    void operator()();

    ///
    /// @brief Flushes the output stream.
    ///
    void flush();

    ///
    /// @brief Formats a number of arbitrarily-typed arguments into a string.
    ///
    /// @param[in] kFmt   Format string. The format specifier is "%%",
    ///                   regardless of type.
    /// @param[in] kArgs  Format arguments. Each argument will be converted to
    ///                   a string by left-shifting it into a stream, so this
    ///                   operator must be defined for the argument type.
    ///
    /// @returns Formatted string.
    ///
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

    ///
    /// @brief Output stream.
    ///
    std::ostream& mOs;

    ///
    ///
    /// @brief Current indent level.
    U32 mIndentLvl;

    ///
    /// @brief Recursive variadic template for string formatting.
    ///
    /// @param[in] kFmt   Format string.
    /// @param[in] kT     Current argument to format.
    /// @param[in] kArgs  Remaining arguments to format.
    ///
    /// @returns Formatted string in progress.
    ///
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

    ///
    /// @brief Base case for recursive formatting template.
    ///
    /// @param[in] kStr  Format string.
    ///
    /// @returns Format string, unchanged.
    ///
    static String& formatStep(String& kStr);
};

#endif
