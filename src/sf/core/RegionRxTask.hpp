#ifndef SF_REGION_RX_TASK_HPP
#define SF_REGION_RX_TASK_HPP

#include "sf/core/Region.hpp"
#include "sf/core/Task.hpp"
#include "sf/pal/Socket.hpp"

template<U32 TCnt>
class RegionRxTask final : public ITask
{
public:

    struct Config final
    {
        Socket* socks[TCnt];
        Region* regions[TCnt];
        Element<U32>* elemsRxCnt[TCnt];
        Element<U32>* elemsAge[TCnt];
        U32 timeoutUs;
        U32 additionalPolls;
    };

    RegionRxTask(const Element<U8>* const kElemMode,
                 const RegionRxTask::Config kConfig) :
        ITask(kElemMode), mConfig(kConfig)
    {
    }

private:

    ///
    /// @brief Task config.
    ///
    const RegionRxTask::Config mConfig;

    Result initImpl() final override
    {
        for (U32 i = 0; i < TCnt; ++i)
        {
            if ((mConfig.socks[i] == nullptr)
                || (mConfig.regions[i] == nullptr))
            {
                return E_RRX_NULL;
            }
        }

        return SUCCESS;
    }

    Result stepEnable() final override
    {
        SF_SAFE_ASSERT(mConfig.socks != nullptr);
        SF_SAFE_ASSERT(mConfig.regions != nullptr);

        Result res = SUCCESS;
        (void) res;

        // Remaining rx time in microseconds, decreased as Socket::select()
        // calls are made.
        U32 timeoutUs = mConfig.timeoutUs;

        // Socket ready flags. Flags are not reset between Socket::select()
        // calls, so each socket will be read at most once.
        bool ready[TCnt] = {};

        // Region receive flags. Any time a region is received, the
        // corresponding index in this array will be set to true.
        bool recvdRegions[TCnt] = {};

        // Loop until timeout expires.
        do
        {
            // Wait for a socket to have data available.
            res = Socket::select(mConfig.socks, ready, TCnt, timeoutUs);
            if (res != SUCCESS)
            {
                return res;
            }

            // Try to read data from sockets.
            res = this->tryReadSockets(ready, recvdRegions);
            if (res != SUCCESS)
            {
                return res;
            }
        }
        while (timeoutUs > 0);

        // With the timeout depleted, do additional polls on sockets as
        // configured, e.g., to make sure data isn't building up in rx queues.
        for (U32 i = 0; i < mConfig.additionalPolls; ++i)
        {
            // Call Socket::select() with timeout 0 and all ready flags false so
            // that every socket is polled.
            bool readyBlank[TCnt] = {};
            U32 pollTimeout = 0;
            res = Socket::select(mConfig.socks, readyBlank, TCnt, pollTimeout);
            if (res != SUCCESS)
            {
                return res;
            }

            // Try to read data from sockets.
            res = this->tryReadSockets(ready, recvdRegions);
            if (res != SUCCESS)
            {
                return res;
            }
        }

        // Update configured rx count and age elements.
        for (U32 i = 0; i < TCnt; ++i)
        {
            if ((mConfig.elemsRxCnt[i] != nullptr) && (recvdRegions[i]))
            {
                // Region was receive this step, so increment rx count.
                mConfig.elemsRxCnt[i]->write(mConfig.elemsRxCnt[i]->read() + 1);
            }

            if (mConfig.elemsAge[i] != nullptr)
            {
                if (recvdRegions[i])
                {
                    // Region was received this step, so set age back to 0.
                    mConfig.elemsAge[i]->write(0);
                }
                else
                {
                    // Region was not received this step, so increment age.
                    mConfig.elemsAge[i]->write(mConfig.elemsAge[i]->read() + 1);
                }
            }
        }

        return SUCCESS;
    }

    Result tryReadSockets(const bool kReady[TCnt], bool kRecvdRegions[TCnt])
    {
        // Loop through sockets.
        for (U32 i = 0; i < TCnt; ++i)
        {
            if (kReady[i])
            {
                // Socket has data available.

                SF_SAFE_ASSERT(mConfig.socks[i] != nullptr);
                SF_SAFE_ASSERT(mConfig.regions[i] != nullptr);

                Socket& sock = *mConfig.socks[i];
                Region& region = *mConfig.regions[i];

                // Read from socket into region.
                U32 totalBytesRecvd = 0;
                while (totalBytesRecvd < region.size())
                {
                    // Compute address in region to start read at and number of
                    // bytes to read.
                    U8* const recvAddr = (static_cast<U8*>(region.addr())
                                          + totalBytesRecvd);
                    const U32 bytesToRecv = (region.size() - totalBytesRecvd);

                    // Do read.
                    U32 bytesRecvd = 0;
                    const Result res = sock.recv(recvAddr,
                                                 bytesToRecv,
                                                 &bytesRecvd);
                    if (res != SUCCESS)
                    {
                        return res;
                    }

                    totalBytesRecvd += bytesRecvd;
                }

                // Update received regions array.
                kRecvdRegions[i] = true;
            }
        }

        return SUCCESS;
    }
};

#endif
