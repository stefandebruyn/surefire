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
///
///                             ---------------
/// @file  sf/core/Region.hpp
/// @brief State vector region object.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_REGION_HPP
#define SF_REGION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"
#include "sf/pal/Lock.hpp"

///
/// @brief A region represents the backing memory of a contiguous set of state
/// vector elements.
///
/// @remark Using regions in conjunction with a state vector is optional. They
/// are intended for bulk state vector reads and writes involving raw memory
/// buffers and may be used, e.g., to format network packets.
///
/// @see Element
/// @see StateVector
///
class Region final
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @remark The caller assumes responsibility for validating the region
    /// address and size. Ideally the region exactly spans the backing for some
    /// number of state vector elements and does not overlap with other regions.
    ///
    /// @param[in] kAddr       Region address.
    /// @param[in] kSizeBytes  Region size in bytes.
    ///
    Region(void* const kAddr, const U32 kSizeBytes);

    ///
    /// @brief Constructor for a thread-safe region.
    ///
    /// @remark The caller assumes responsibility for validating the region
    /// address and size. Ideally the region exactly spans the backing for some
    /// number of state vector elements and does not overlap with other regions.
    ///
    /// @param[in] kAddr       Region address.
    /// @param[in] kSizeBytes  Region size in bytes.
    /// @param[in] kLock       Lock to be acquired on every region access.
    ///
    Region(void* const kAddr, const U32 kSizeBytes, ILock* const kLock);

    ///
    /// @brief Overwrites the entire region.
    ///
    /// @param[in] kBuf           Address of buffer to overwrite region with.
    /// @param[in] kBufSizeBytes  Buffer size in bytes. This must exactly match
    ///                           the region size.
    ///
    /// @retval SUCCESS     Successfully overwrote region.
    /// @retval E_RGN_SIZE  Buffer size did not match region size.
    ///
    Result write(const void* const kBuf, const U32 kBufSizeBytes);

    ///
    /// @brief Reads the entire region into a buffer.
    ///
    /// @param[in] kBuf           Buffer to read into.
    /// @param[in] kBufSizeBytes  Buffer size in bytes. This must exactly match
    ///                           the region size.
    ///
    /// @retval SUCCESS     Successfully read region into buffer.
    /// @retval E_RGN_SIZE  Buffer size did not match region size.
    ///
    Result read(void* const kBuf, const U32 mBufSizeBytes) const;

    ///
    /// @brief Gets the address of the region backing.
    ///
    /// @return Region backing address.
    ///
    void* addr() const;

    ///
    /// @brief Gets the size of the region in bytes.
    ///
    /// @return Region size.
    ///
    U32 size() const;

    Region(const Region&) = delete;
    Region(Region&&) = delete;
    Region& operator=(const Region&) = delete;
    Region& operator=(Region&&) = delete;

private:

    ///
    /// @brief Region backing address.
    ///
    void* const mAddr;

    ///
    /// @brief Region size in bytes.
    ///
    const U32 mSizeBytes;

    ///
    /// @brief Region lock, or null if none.
    ///
    ILock* const mLock;
};

#endif
