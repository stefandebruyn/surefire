#include <Arduino.h>

#include "sfa/pal/Clock.hpp"

U64 Clock::nanoTime()
{
    return (micros() * static_cast<U64>(1000));
}
