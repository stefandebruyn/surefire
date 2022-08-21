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
/// @file  sf/cli/StateVectorCommand.hpp
/// @brief CLI state vector command implementations.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_VECTOR_COMMAND_HPP
#define SF_STATE_VECTOR_COMMAND_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"

namespace Sf
{

namespace Cli
{
    ///
    /// @brief Top-level state vector command entry point.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sv`.
    ///
    /// @returns Exit status.
    ///
    I32 sv(const Vec<String> kArgs);

    ///
    /// @brief State vector config check command.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sv check`.
    ///
    /// @returns Exit status.
    ///
    I32 svCheck(const Vec<String> kArgs);

    ///
    /// @brief State vector autocode command.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sv autocode`.
    ///
    /// @returns Exit status.
    ///
    I32 svAutocode(const Vec<String> kArgs);
}

} // namespace Sf

#endif
