#ifndef SF_DIGITAL_IO_HPP
#define SF_DIGITAL_IO_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

class DigitalIO final
{
public:

    enum Mode : U8
    {
        IN = 0,
        OUT = 1
    };

    static Result create(DigitalIO& kDio);

    DigitalIO();

    ~DigitalIO();

    Result setMode(const U32 kPin, const Mode kMode);

    Result read(const U32 kPin, bool& kVal);

    Result write(const U32 kPin, const bool kVal);

    Result close();

private:

    bool mInit;

#ifdef SF_PLATFORM_ARDUINO
    /// Bit vector of pin output values. The rightmost bit stores the last value
    /// written to pin 0, the 2nd rightmost stores pin 1, and so on. This is
    /// used to lower all pins raised by the DIO object when the object is
    /// closed/destructed.
    U64 mOutBitVec;
#endif
};

#endif
