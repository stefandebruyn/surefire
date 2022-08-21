////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Diagnostic.hpp"
#include "sf/pal/Console.hpp"
#include "sf/pal/System.hpp"

namespace Sf
{

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
        Diag::printOnError(kRes, kMsg);
        Console::printf("Halting...\n");
        System::exit(1);
    }
}

} // namespace Sf
