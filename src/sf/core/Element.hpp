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
/// @file   sf/core/Element.hpp
/// @brief  State vector element interface.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ELEMENT_HPP
#define SF_ELEMENT_HPP

#include "sf/core/BasicTypes.hpp"

///
/// @brief Enumeration of possible state vector element data types.
///
enum ElementType : U32
{
    INT8 = 0,
    INT16 = 1,
    INT32 = 2,
    INT64 = 3,
    UINT8 = 4,
    UINT16 = 5,
    UINT32 = 6,
    UINT64 = 7,
    FLOAT32 = 8,
    FLOAT64 = 9,
    BOOL = 10
};

///
/// @brief Abstract interface for a state vector element.
///
/// @remark The main purpose for this interface is to make `Element` template
/// instantiations polymorphic.
///
class IElement
{
public:

    ///
    /// @brief Destructor.
    ///
    virtual ~IElement() = default;

    ///
    /// @brief Gets the element type.
    ///
    /// @remark This method is mainly used by the framework config library
    /// (which often operates on containers of mixed-type elements) for RTTI.
    ///
    /// @return Element type.
    ///
    virtual ElementType type() const = 0;

    ///
    /// @brief Gets the address of the element backing.
    ///
    /// @warning This is for testing purposes and should never be used to read
    /// or write the element.
    ///
    /// @return Element backing address.
    ///
    virtual const void* addr() const = 0;

    ///
    /// @brief Gets the size of the element type in bytes.
    ///
    /// @return Element size.
    ///
    virtual U32 size() const = 0;
};

///
/// @brief An element is a strongly-typed variable in a state vector.
///
/// An element object acts like a handle to a typed piece of memory. "Element"
/// refers to the handle, and "backing" refers to the underlying memory.
///
/// @remark Elements are non-copyable and non-movable. Elements should not share
/// backing. Objects which use the same element share a reference to it.
///
/// @remark The framework state vector component (and other components which use
/// it, e.g., the state machine) only support the primitive element types
/// enumerated in `ElementType`.
///
/// @tparam T  Element type.
///
template<typename T>
class Element final : public IElement
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @remark The element backing should be inaccessible to anything which is
    /// not the element. The backing must live at least as long as the element
    /// it backs.
    ///
    /// @param[in] backing  Element backing.
    ///
    Element(T& kBacking) : mBacking(kBacking)
    {
    }

    ///
    /// @brief Sets the element value.
    ///
    /// @param[in] kVal  Write value.
    ///
    void write(const T kVal) const
    {
        mBacking = kVal;
    }

    ///
    /// @brief Gets the element value.
    ///
    /// @return Read value.
    ///
    T read() const
    {
        return mBacking;
    }

    ///
    /// @brief See `IElement::type`.
    ///
    ElementType type() const final override;

    ///
    /// @brief See `IElement::addr`.
    ///
    const void* addr() const final override
    {
        return &mBacking;
    }

    ///
    /// @brief See `IElement::size`.
    ///
    U32 size() const final override
    {
        return sizeof(T);
    }

    Element(const Element<T>&) = delete;
    Element(Element<T>&&) = delete;
    Element<T>& operator=(const Element<T>&) = delete;
    Element<T>& operator=(Element<T>&&) = delete;

private:

    ///
    /// @brief Element backing.
    ///
    T& mBacking;
};

#endif
