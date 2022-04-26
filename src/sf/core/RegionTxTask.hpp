////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Built in Austin, Texas at the University of Texas at Austin.
/// Surefire is open-source under the Apache License 2.0 - a copy of the license
/// may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/core/RegionTxTask.hpp
/// @brief Task which sends a state vector region over the network.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_REGION_TX_TASK_HPP
#define SF_REGION_TX_TASK_HPP

#include "sf/core/Region.hpp"
#include "sf/core/Task.hpp"
#include "sf/pal/Socket.hpp"

///
/// @brief Task which sends a state vector region over the network.
///
/// When the task is enabled, the region is sent once per step. The task does
/// nothing when safed.
///
class RegionTxTask final : public ITask
{
public:

    ///
    /// @brief Task configuration parameters.
    ///
    struct Config final
    {
        ///
        /// @brief Socket to send from.
        ///
        /// @note When sending a region the task will call Socket::send() in a
        /// loop until the entire region is sent, so it is safe to use a socket
        /// with a protocol that may not send all at once.
        ///
        Socket& sock;

        ///
        /// @brief Region to send.
        ///
        Region& region;

        ///
        /// @brief IP to send to.
        ///
        IPv4Address destIp;

        ///
        /// @brief Port to send to.
        ///
        U16 destPort;

        ///
        /// @brief Optional element to increment on successful region send.
        ///
        Element<U32>* elemTxCnt;
    };

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kElemMode  Pointer to optional task mode element.
    /// @param[in] kConfig    Task config.
    ///
    RegionTxTask(const Element<U8>* const kElemMode,
                 const RegionTxTask::Config kConfig);

private:

    ///
    /// @brief Task config.
    ///
    const RegionTxTask::Config mConfig;

    ///
    /// @brief Task initialization logic.
    ///
    /// @retval SUCCESS  Always succeeds.
    ///
    Result initImpl() final override;

    ///
    /// @brief Sends the configured region to the configured address once.
    ///
    /// @retval SUCCESS     Successfully sent region. If a tx count element was
    ///                     configured, the element was incremented.
    /// @retval E_RTX_SIZE  Send succeeded but the number of bytes sent did not
    ///                     match the region size.
    ///
    Result stepEnable() final override;
};

#endif
