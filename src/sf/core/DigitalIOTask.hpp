#ifndef SF_DIGITAL_IO_TASK_HPP
#define SF_DIGITAL_IO_TASK_HPP

#include "sf/core/Task.hpp"
#include "sf/pal/DigitalIO.hpp"

class DigitalIOTask final : public ITask
{
public:

    struct Channel final
    {
        DigitalIO::Mode direction;
        U32 pin;
        Element<bool>* elem;
    };

    struct Config final
    {
        Channel* channels;
    };

    DigitalIOTask(const Element<U8>* const kElemMode,
                  const DigitalIOTask::Config kConfig);

private:

    const DigitalIOTask::Config mConfig;

    DigitalIO mDio;

    Result initImpl() final override;

    Result stepEnable() final override;
};

#endif
