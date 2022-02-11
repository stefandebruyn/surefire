#ifndef SFA_DIO_HPP
#define SFA_DIO_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"

class Dio final
{
public:

    enum Mode : U8
    {
        MODE_INPUT,
        MODE_OUTPUT
    };

    static Result create(Dio& kDio);

    Dio();

    ~Dio();

    Result setMode(const U32 kPin, const Mode kMode);

    Result read(const U32 kPin, bool& kVal);

    Result write(const U32 kPin, const bool kVal);

    Result close();

private:

    bool mInit;

#ifdef SFA_PLATFORM_ARDUINO
    /// Bit vector of pin output values. The rightmost bit stores the last value
    /// written to pin 0, the 2nd rightmost stores pin 1, and so on. This is
    /// used to lower all pins raised by the DIO object when the object is
    /// closed/destructed.
    U64 mOutBitVec;
#endif
};

#endif
