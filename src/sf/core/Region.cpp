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

#include "sf/core/Assert.hpp"
#include "sf/core/Region.hpp"
#include "sf/core/MemOps.hpp"

namespace Sf
{

Region::Region(void* const kAddr, const U32 kSizeBytes) :
    Region(kAddr, kSizeBytes, nullptr)
{
}

Region::Region(void* const kAddr, const U32 kSizeBytes, ILock* const kLock) :
    mAddr(kAddr), mSizeBytes(kSizeBytes), mLock(kLock)
{
}

Result Region::write(const void* const kBuf, const U32 kBufSizeBytes)
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_RGN_SIZE;
    }

    if (mLock != nullptr)
    {
        // Acquire region lock.
        const Result res = mLock->acquire();
        if (res != SUCCESS)
        {
            return res;
        }
    }

    MemOps::memcpy(mAddr, kBuf, mSizeBytes);

    if (mLock != nullptr)
    {
        // Release region lock.
        const Result res = mLock->release();
        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

Result Region::read(void* const kBuf, const U32 kBufSizeBytes) const
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_RGN_SIZE;
    }

    if (mLock != nullptr)
    {
        // Acquire region lock.
        const Result res = mLock->acquire();
        (void) res;
        SF_ASSERT(res == SUCCESS);
    }

    MemOps::memcpy(kBuf, mAddr, mSizeBytes);

    if (mLock != nullptr)
    {
        // Release region lock.
        const Result res = mLock->release();
        (void) res;
        SF_ASSERT(res == SUCCESS);
    }

    return SUCCESS;
}

void* Region::addr() const
{
    return mAddr;
}

U32 Region::size() const
{
    return mSizeBytes;
}

} // namespace Sf
