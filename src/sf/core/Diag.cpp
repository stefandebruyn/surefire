#include "sf/core/Diag.hpp"
#include "sf/pal/Console.hpp"
#include "sf/pal/System.hpp"

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
        System::exit(1);
    }
}