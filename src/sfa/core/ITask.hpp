#ifndef SFA_ITASK_HPP
#define SFA_ITASK_HPP

#include "sfa/core/StateVector.hpp"
#include "sfa/core/Element.hpp"
#include "sfa/core/Result.hpp"

enum TaskMode : U8
{
    MODE_DISABLE,
    MODE_SAFE,
    MODE_ENABLE
};

class ITask
{
public:

    constexpr ITask(StateVector& kSv, const Element<U8>* kModeElem) :
        mSv(kSv), mModeElem(kModeElem), mInitialized(false)
    {
    }

    virtual ~ITask() = default;

    virtual Result initialize() final;

    virtual Result step() final;

protected:

    StateVector& mSv;

    virtual Result initializeImpl() = 0;

    virtual Result stepSafe();

    virtual Result stepEnable() = 0;

private:

    const Element<U8>* mModeElem;

    bool mInitialized;
};

#endif
