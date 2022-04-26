////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Built in Austin, Texas at the University of Texas at Austin.
/// Surefire is open-source under the Apache License 2.0 - a copy of the license
/// may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
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
