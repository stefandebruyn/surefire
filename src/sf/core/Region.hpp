////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/Region.hpp
/// @brief State vector region object.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_REGION_HPP
#define SF_REGION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

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
};

#endif
