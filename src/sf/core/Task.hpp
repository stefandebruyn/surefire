#ifndef SF_ITASK_HPP
#define SF_ITASK_HPP

#include "sf/core/Element.hpp"
#include "sf/core/Result.hpp"
#include "sf/core/StateVector.hpp"

enum TaskMode : U8
{
    DISABLE = 0,
    SAFE = 1,
    ENABLE = 2
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
