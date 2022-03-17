#include <cstdlib>
#include <iostream>

#include "sf/pal/System.hpp"

void System::exit(const I32 kStatus)
{
    ::exit(kStatus);
}
