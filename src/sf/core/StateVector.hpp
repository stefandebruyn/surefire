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
/// @file  sf/core/StateVector.hpp
/// @brief State vector object.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_VECTOR_HPP
#define SF_STATE_VECTOR_HPP

#include "sf/core/Assert.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Region.hpp"
#include "sf/core/Result.hpp"

namespace Sf
{

///
/// @brief A state vector is a collection of named state vector elements and
/// regions. The StateVector object serves only as a lookup table for elements
/// and regions, and is decoupled from the backing memory.
///
/// @remark The user is not meant to manually create a StateVector; it should
/// be the product of an autocoder of compiler in the framework config library.
///
/// @see Element
/// @see Region
///
class StateVector final
{
public:

    ///
    /// @brief Configuration for an element.
    ///
    struct ElementConfig final
    {
        ///
        /// @brief Element name.
        ///
        const char* name;

        ///
        /// @brief Pointer to element object.
        ///
        IElement* elem;
    };

    ///
    /// @brief Configuration for a region.
    ///
    struct RegionConfig final
    {
        ///
        /// @brief Region name.
        ///
        const char* name;

        ///
        /// @brief Pointer to region object.
        ///
        Region* region;
    };

    ///
    /// @brief Configuration for a state vector.
    ///
    struct Config final
    {
        ///
        /// @brief Array of element configs. The array must be terminated with
        /// a null (all-zero) element config.
        ///
        /// @warning Failing to null-terminate the array has undefined behavior.
        ///
        ElementConfig* elems;

        ///
        /// @brief Array of region configs, or null if not using regions. If
        /// non-null, the array must be terminated with a null (all-zero) region
        /// config. If non-null, all configured regions must be contiguous and
        /// exactly span the backing of all configured elements.
        ///
        /// @warning Failing to null-terminate the array has undefined behavior.
        ///
        RegionConfig* regions;
    };

    ///
    /// @brief Initializes a state vector from a config.
    ///
    /// @warning A StateVector exists separately from the config. The config is
    /// not copied. The config and all data therein must live at least as long
    /// as the StateVector. Modifying the config after using it to initialize a
    /// StateVector has undefined behavior. The same config should not be used
    /// to initialize more than one StateVector.
    ///
    /// @param[in] kConfig  State vector config.
    /// @param[in] kSv      State vector to initialize.
    ///
    /// @retval SUCCESS         Successfully initialized state vector.
    /// @retval E_SV_REINIT     State vector is already initialized.
    /// @retval E_SV_NULL       Config contains a null element or region object
    ///                         pointer.
    /// @retval E_SV_ELEM_DUPE  Duplicate element name.
    /// @retval E_SV_RGN_DUPE   Duplicate region name.
    /// @retval E_SV_LAYOUT     Regions are not contiguous or do not exactly
    ///                         span element backing.
    ///
    static Result init(const Config kConfig, StateVector& kSv);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed StateVector is uninitialized and invoking any of
    /// its methods returns an error.
    ///
    StateVector();

    ///
    /// @brief Gets a pointer to an element object by name.
    ///
    /// @post On success, kElem contains a pointer to the specified element.
    /// @post On error, kElem is unchanged.
    ///
    /// @tparam T  Element data type.
    ///
    /// @param[in]  kName  Element name.
    /// @param[out] kElem  On success, assigned pointer to requested element.
    ///
    /// @retval SUCCESS      Successfully got element.
    /// @retval E_SV_UNINIT  State vector is not initialized.
    /// @retval E_SV_KEY     Unknown element.
    /// @retval E_SV_TYPE    Element exists but does not have the expected type.
    ///
    template<typename T>
    Result getElement(const char* const kName, Element<T>*& kElem);

    ///
    /// @brief Gets an abstract pointer to an element object by name.
    ///
    /// @post On success, kElem contains a pointer to the specified element.
    /// @post On error, kElem is unchanged.
    ///
    /// @remark This method is useful for checking element existence, as it does
    /// not require that the specified element be a particular type.
    ///
    /// @param[in]  kName  Element name.
    /// @param[out] kElem  Reference to assign pointer to element.
    ///
    /// @retval SUCCESS      Successfully got element.
    /// @retval E_SV_UNINIT  State vector is not initialized.
    /// @retval E_SV_KEY     Unknown element.
    ///
    Result getIElement(const char* const kName, IElement*& kElem);

    ///
    /// @brief Gets a pointer to a region object by name.
    ///
    /// @post On success, kRegion contains a pointer to the specified region.
    /// @post On error, kRegion is unchanged.
    ///
    /// @param[in]  kName    Region name.
    /// @param[out] kRegion  Reference to assign pointer to region.
    ///
    /// @retval SUCCESS      Successfully got region.
    /// @retval E_SV_UNINIT  State vector is not initialized.
    /// @retval E_SV_EMPTY   State vector was not configured with regions.
    /// @retval E_SV_KEY     Unknown region.
    ///
    Result getRegion(const char* const kName, Region*& kRegion);

    StateVector(const StateVector&) = delete;
    StateVector(StateVector&&) = delete;
    StateVector& operator=(const StateVector&) = delete;
    StateVector& operator=(StateVector&&) = default;

private:

    ///
    /// @brief State vector config. When the pointers in the config are null,
    /// the state vector is uninitialized; the state vector factory method
    /// initializes the state vector by setting these pointers.
    ///
    StateVector::Config mConfig;

    ///
    /// @brief Looks up an element config by name.
    ///
    /// @param[in] kName         Element name.
    /// @param[out] kElemConfig  On success, assigned pointer to requested
    ///                          config.
    ///
    /// @retval SUCCESS   Successfully got config.
    /// @retval E_SV_KEY  Unknown element.
    ///
    Result getElementConfig(const char* const kName,
                            const ElementConfig*& kElemConfig) const;

    ///
    /// @brief Looks up a region config by name.
    ///
    /// @param[in]  kName        Region name.
    /// @param[out] kElemConfig  On success, assigned pointer to requested
    ///                          config.
    ///
    /// @retval SUCCESS   Successfully got config.
    /// @retval E_SV_KEY  Unknown region.
    ///
    Result getRegionConfig(const char* const kName,
                           const RegionConfig*& kRegionConfig) const;

    ///
    /// @brief Element lookup helper with type checking.
    ///
    /// @param[in]  kName      Element name.
    /// @param[out] kElem      On success, assigned pointer to requested
    ///                        element.
    /// @param[in]  kElemType  Expected element type.
    ///
    ///
    /// @retval SUCCESS      Successfully got element.
    /// @retval E_SV_UNINIT  State vector is not initialized.
    /// @retval E_SV_KEY     Unknown element.
    /// @retval E_SV_TYPE    Element exists but does not have the expected type.
    ///
    template<typename T>
    Result getElementImpl(const char* const kName,
                          Element<T>*& kElem,
                          const ElementType kElemType)
    {
        // Check that state vector is initialized.
        if (mConfig.elems == nullptr)
        {
            return E_SV_UNINIT;
        }

        // Look up element config.
        const ElementConfig* elemConfig = nullptr;
        const Result res = this->getElementConfig(kName, elemConfig);
        if (res != SUCCESS)
        {
            return res;
        }

        SF_SAFE_ASSERT(elemConfig != nullptr);
        SF_SAFE_ASSERT(elemConfig->elem != nullptr);

        // Check that element has the expected type.
        IElement* const elem = elemConfig->elem;
        if (elem->type() != kElemType)
        {
            return E_SV_TYPE;
        }

        // Lookup is valid - downcast element pointer to template instantiation.
        kElem = static_cast<Element<T>*>(elem);

        return SUCCESS;
    }
};

} // namespace Sf

#endif
