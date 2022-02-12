#ifndef SFA_ANALOG_IO_HPP
#define SFA_ANALOG_IO_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"

class AnalogIo final
{
public:

    static Result create(AnalogIo& kAio);

    AnalogIo();

    ~AnalogIo();

    Result setRange(const U32 kPin, const I8 kRange);

    Result read(const U32 kPin, F32& kVal);

    Result write(const U32 kPin, const F32 kVal);

    Result close();

private:

    bool mInit;
};

#endif
