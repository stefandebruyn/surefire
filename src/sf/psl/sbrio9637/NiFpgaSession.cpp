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
////////////////////////////////////////////////////////////////////////////////

#include "NiFpgaSession.hpp"
#include "sf/core/Assert.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Spinlock.hpp"

namespace Sf
{

///
/// @brief Lock for synchronizing FGPA session management - opening sessions,
/// closing sessions, updating the session count, etc.
///
/// @note The FPGA API specifically notes that the functions NiFpga_Initialize()
/// and NiFpga_Finalize() are not thread-safe.
///
static Spinlock gLock;

///
/// @brief Number of open FPGA sessions.
///
static U32 gOpenSessionCnt = 0;

Result niFpgaSessionOpen(NiFpga_Session& kSession)
{
    // Whether this function has been called or not. The first call will
    // initialize the lock and FPGA API.
    static bool apiInit = false;

    // Initialize lock on the first call to this function.
    if (!apiInit)
    {
        const Result res = Spinlock::init(gLock);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // FPGA API error code.
    NiFpga_Status stat = NiFpga_Status_Success;

    // Acquire the lock.
    SF_SAFE_ASSERT(gLock.acquire() == SUCCESS);

    // Initialize the FPGA API on the first call to this function.
    if (!apiInit)
    {
        // Initialize FPGA API.
        stat = NiFpga_Initialize();
        if (stat != NiFpga_Status_Success)
        {
            // Release lock but disregard any errors since the FPGA error is
            // more important.
            (void) gLock.release();
            return E_NI_FPGA_INIT;
        }

        apiInit = true;
    }

    // Check that maximum number of FPGA sessions has not been reached. The max
    // is based on a known bug in the FPGA API that causes a crash if too many
    // sessions are open at once:
    // https://www.ni.com/en-us/support/documentation/bugs/19/labview-2019-fpga-module-known-issues.html#733478_by_Date
    static constexpr U32 maxSessions = 127;
    if (gOpenSessionCnt >= maxSessions)
    {
        // Release lock but disregard any errors since the FPGA error is more
        // important.
        (void) gLock.release();
        return E_NI_FPGA_MAX;
    }

    // Open a new FPGA session.
    NiFpga_Session session = 0;
    NiFpga_MergeStatus(&stat, NiFpga_Open(NiFpga_IO_Bitfile,
                                          NiFpga_IO_Signature,
                                          "RIO0",
                                          0,
                                          &session));
    if (stat != NiFpga_Status_Success)
    {
        // Release lock but disregard any errors since the FPGA error is more
        // important.
        (void) gLock.release();
        return E_NI_FPGA_OPEN;
    }

    // Wait a relatively long time to avoid racing FPGA initialization.
    Clock::spinWait(Clock::NS_IN_S);

    // Increment open FPGA session count.
    ++gOpenSessionCnt;

    // Release lock.
    SF_SAFE_ASSERT(gLock.release() == SUCCESS);

    // Return FPGA session handle.
    kSession = session;

    return SUCCESS;
}

Result niFpgaSessionClose(const NiFpga_Session kSession)
{
    // Close FPGA session. No need to acquire the lock yet since NiFpga_Close()
    // is thread-safe.
    NiFpga_Status stat = NiFpga_Status_Success;
    NiFpga_MergeStatus(&stat, NiFpga_Close(kSession, 0));
    if (stat != NiFpga_Status_Success)
    {
        return E_NI_FPGA_CLOSE;
    }

    // Decrement open FPGA session count.
    SF_SAFE_ASSERT(gLock.acquire() == SUCCESS);
    --gOpenSessionCnt;
    SF_SAFE_ASSERT(gLock.release() == SUCCESS);

    return SUCCESS;
}

} // namespace Sf
