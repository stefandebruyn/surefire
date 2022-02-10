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
};

#endif
