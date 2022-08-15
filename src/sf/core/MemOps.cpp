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

#include "sf/core/MemOps.hpp"

I32 MemOps::strcmp(const char* kA, const char* kB)
{
    I32 ret = 0;

    if ((kA != nullptr) != (kB != nullptr))
    {
        // One string is null, but not both - treat the null string like an
        // empty string.
        if (kA == nullptr)
        {
            ret = -kB[0];
        }
        else
        {
            ret = kA[0];
        }
    }
    else if ((kA != nullptr) && (kB != nullptr))
    {
        // Both strings are non-null - scan through for the first non-matching
        // character and return their difference.
        do
        {
            ret = (*kA - *kB);
            if (ret != 0)
            {
                break;
            }
        }
        while ((*kA++ != '\0') && (*kB++ != '\0'));
    }

    return ret;
}

void* MemOps::memcpy(void* kDest, const void* const kSrc, const U32 kBytes)
{
    char* const dest = static_cast<char*>(kDest);
    const char* const src = static_cast<const char*>(kSrc);

    // TODO: This could be made more efficient by moving >1 byte at once...
    if ((dest != nullptr) && (src != nullptr))
    {
        for (U32 i = 0; i < kBytes; ++i)
        {
            *(dest + i) = *(src + i);
        }
    }

    return dest;
}
