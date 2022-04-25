#include "sf/core/AnalogIOTask.hpp"

AnalogIOTask::AnalogIOTask(const Element<U8>* const kElemMode,
                             const AnalogIOTask::Config kConfig) :
    ITask(kElemMode), mConfig(kConfig)
{
}

Result AnalogIOTask::initImpl()
{
    // Check that channels are non-null.
    if (mConfig.channels == nullptr)
    {
        return E_AIT_NULL;
    }

    // Initialize AnalogIO.
    Result res = AnalogIO::init(mAio);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set pin parameters.
    for (const AnalogIOTask::Channel* chan = mConfig.channels;
         chan->elem != nullptr;
         ++chan)
    {
        // TODO
    }

    return SUCCESS;
}

Result AnalogIOTask::stepEnable()
{
    for (const AnalogIOTask::Channel* chan = mConfig.channels;
         chan->elem != nullptr;
         ++chan)
    {
        if (chan->input)
        {
            // Read input pin.
            F32 val = 0.0f;
            const Result res = mAio.read(chan->pin, val);
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
            const F32 val = chan->elem->read();

            // Write output pin.
            const Result res = mAio.write(chan->pin, val);
            if (res != SUCCESS)
            {
                return res;
            }
        }
    }

    return SUCCESS;
}
