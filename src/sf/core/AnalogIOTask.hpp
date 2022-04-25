#ifndef SF_ANALOG_IO_TASK_HPP
#define SF_ANALOG_IO_TASK_HPP

#include "sf/core/Task.hpp"
#include "sf/pal/AnalogIO.hpp"

class AnalogIOTask final : public ITask
{
public:

    struct Channel final
    {
        bool input;
        U32 pin;
        Element<F32>* elem;
    };

    struct Config final
    {
        Channel* channels;
    };

    AnalogIOTask(const Element<U8>* const kElemMode,
                 const AnalogIOTask::Config kConfig);

private:

    const AnalogIOTask::Config mConfig;

    AnalogIO mAio;

    Result initImpl() final override;

    Result stepEnable() final override;
};

#endif
