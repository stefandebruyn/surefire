#include <cstdlib>
#include <iostream>

#include "sfa/pal/System.hpp"

void System::exit(const I32 kStatus)
{
    ::exit(kStatus);
}
