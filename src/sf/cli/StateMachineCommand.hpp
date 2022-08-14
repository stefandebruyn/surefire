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
///
///                             ---------------
/// @file  sf/cli/StateMachineCommand.hpp
/// @brief CLI state machine command implementations.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_MACHINE_COMMAND_HPP
#define SF_STATE_MACHINE_COMMAND_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"

namespace Cli
{
    ///
    /// @brief Top-level state machine command entry point.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sm`.
    ///
    /// @returns Exit status.
    ///
    I32 sm(const Vec<String> kArgs);

    ///
    /// @brief State machine config check command.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sm check`.
    ///
    /// @returns Exit status.
    ///
    I32 smCheck(const Vec<String> kArgs);

    ///
    /// @brief State machine test command.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sm test`.
    ///
    /// @returns Exit status.
    ///
    I32 smTest(const Vec<String> kArgs);

    ///
    /// @brief State machine autocode command.
    ///
    /// @param[in] kArgs  Command arguments, starting with the first argument
    ///                   after `sm autocode`.
    ///
    /// @returns Exit status.
    ///
    I32 smAutocode(const Vec<String> kArgs);
}

#endif
