#include <Arduino.h>

#include "sfa/pal/Clock.hpp"

U64 Clock::nanoTime()
{
    return (micros() * 1000);
}
