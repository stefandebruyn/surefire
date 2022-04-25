#include "sf/core/DigitalIOTask.hpp"

DigitalIOTask::DigitalIOTask(const Element<U8>* const kElemMode,
                             const DigitalIOTask::Config kConfig) :
    ITask(kElemMode), mConfig(kConfig)
{
}

Result DigitalIOTask::initImpl()
{
    // Check that channels are non-null.
    if (mConfig.channels == nullptr)
    {
        return E_DIT_NULL;
    }

    // Initialize DigitalIO.
    Result res = DigitalIO::init(mDio);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set pin modes.
    for (const DigitalIOTask::Channel* chan = mConfig.channels;
         chan->elem != nullptr;
         ++chan)
    {
        res = mDio.setMode(chan->pin, chan->direction);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

Result DigitalIOTask::stepEnable()
{
    for (const DigitalIOTask::Channel* chan = mConfig.channels;
         chan->elem != nullptr;
         ++chan)
    {
        if (chan->direction == DigitalIO::IN)
        {
            // Read input pin.
            bool val = false;
            const Result res = mDio.read(chan->pin, val);
            if (res != SUCCESS)
            {
                return res;
            }

            // Write input value to element.
            chan->elem->write(val);
        }
        else
        {
            // Read output value from element.
            const bool val = chan->elem->read();

            // Write output pin.
            const Result res = mDio.write(chan->pin, val);
            if (res != SUCCESS)
            {
                return res;
            }
        }
    }

    return SUCCESS;
}
