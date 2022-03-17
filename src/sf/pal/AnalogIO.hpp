#ifndef SF_ANALOG_IO_HPP
#define SF_ANALOG_IO_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

class AnalogIO final
{
public:

    static Result create(AnalogIO& kAio);

    AnalogIO();

    ~AnalogIO();

    Result setRange(const U32 kPin, const I8 kRange);

    Result read(const U32 kPin, F32& kVal);

    Result write(const U32 kPin, const F32 kVal);

    Result close();

private:

    bool mInit;
};

#endif
