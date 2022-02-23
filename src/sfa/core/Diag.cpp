#include "sfa/core/Diag.hpp"
#include "sfa/pal/Console.hpp"

void Diag::printOnError(const Result kRes, const char* const kMsg)
{
    if (kRes != SUCCESS)
    {
        Console::printf("[ERROR %d] %s\n", kRes, kMsg);
    }
}

void Diag::haltOnError(const Result kRes, const char* const kMsg)
{
    if (kRes != SUCCESS)
    {
        printOnError(kRes, kMsg);
        Console::printf("Halting...\n");
        while (true);
    }
}