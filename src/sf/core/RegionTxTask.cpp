////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/RegionTxTask.hpp"

RegionTxTask::RegionTxTask(const Element<U8>* const kElemMode,
                           const RegionTxTask::Config kConfig) :
    ITask(kElemMode), mConfig(kConfig)
{
}

Result RegionTxTask::initImpl()
{
    return SUCCESS;
}

Result RegionTxTask::stepEnable()
{
    // Send region to destination address.
    U32 totalBytesSent = 0;
    while (totalBytesSent < mConfig.region.size())
    {
        // Compute address in region to start send at and number of bytes to
        // send.
        const U8* const sendAddr =
            (static_cast<const U8*>(mConfig.region.addr()) + totalBytesSent);
        const U32 bytesToSend = (mConfig.region.size() - totalBytesSent);

        // Do send.
        U32 bytesSent = 0;
        const Result res = mConfig.sock.send(mConfig.destIp,
                                             mConfig.destPort,
                                             sendAddr,
                                             bytesToSend,
                                             &bytesSent);
        if (res != SUCCESS)
        {
            return res;
        }

        totalBytesSent += bytesSent;
    }

    // If a tx count element was provided, increment it.
    if (mConfig.elemTxCnt != nullptr)
    {
        mConfig.elemTxCnt->write(mConfig.elemTxCnt->read() + 1);
    }

    // Return an error if the full region was not sent.
    if (totalBytesSent != mConfig.region.size())
    {
        return E_RTX_SIZE;
    }

    return SUCCESS;
}
