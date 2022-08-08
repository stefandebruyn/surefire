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
/// @file  sf/psl/sbrio9637/NiFpgaSession.hpp
/// @brief Wrapper interface for NiFpga API.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_NI_FPGA_SESSION_HPP
#define SF_NI_FPGA_SESSION_HPP

#include "nifpga/NiFpga.h"
#include "nifpga/NiFpga_IO.h"
#include "sf/core/Result.hpp"

///
/// @brief Opens a new FPGA session.
///
/// @param[in] kSession  On success, set to new session handle.
///
/// @retval SUCCESS         Successfully opened new session.
/// @retval E_NI_FPGA_INIT  Failed to initialize NiFpga API.
/// @retval E_NI_FPGA_MAX   Maximum number of open sessions has been reached.
/// @retval E_NI_FPGA_OPEN  Failed to open session. This usually indicates the
///                         FPGA bitfile is missing or has an invalid signature.
///
Result niFpgaSessionOpen(NiFpga_Session& kSession);

///
/// @brief Closes an FPGA session.
///
/// @param[in] kSession  Session to close.
///
/// @retval SUCCESS          Successfully closed session.
/// @retval E_NI_FPGA_CLOSE  Failed to close session. This may indicate the
///                          session does not exist.
///
Result niFpgaSessionClose(const NiFpga_Session kSession);

#endif
