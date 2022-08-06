////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include <cstdarg>
#include <cstdio>

#include "sf/pal/Console.hpp"

const char* const Console::red = "\033[0;31m";

const char* const Console::yellow = "\033[0;33m";

const char* const Console::green = "\033[0;32m";

const char* const Console::cyan = "\033[0;36m";

const char* const Console::reset = "\033[0m";

I32 Console::printf(const char* const kFmt, ...)
{
    va_list args;
    va_start(args, kFmt);
    const I32 ret = vprintf(kFmt, args);
    va_end(args);
    return ret;
}
