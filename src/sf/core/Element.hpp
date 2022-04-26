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
///
///                             ---------------
/// @file  sf/core/Element.hpp
/// @brief State vector element object.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ELEMENT_HPP
#define SF_ELEMENT_HPP

#include "sf/core/Assert.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/pal/Lock.hpp"

///
/// @brief Enumeration of possible state vector element data types.
///
enum ElementType : U32
{
    NONE = 0,
    INT8 = 1,
    INT16 = 2,
    INT32 = 3,
    INT64 = 4,
    UINT8 = 5,
    UINT16 = 6,
    UINT32 = 7,
    UINT64 = 8,
    FLOAT32 = 9,
    FLOAT64 = 10,
    BOOL = 11
};

///
/// @brief Abstract interface for a state vector element.
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
    /// @return Element type.
    ///
    virtual ElementType type() const = 0;

    ///
    /// @brief Gets the address of the element backing.
    ///
    /// @note This should never be used to read or write the element.
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
/// An Element acts like a handle to a typed piece of memory. "Element" refers
/// to the handle, and "backing" refers to the underlying memory.
///
/// @remark Elements are non-copyable and non-movable. Elements should not share
/// backing. Objects which use the same element share a reference to it.
///
/// @remark StateVector (and other framework components which use it, e.g.,
/// StateMachine) only support the primitive element types enumerated in
/// ElementType.
///
/// @see StateVector
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
    /// @param[in] backing  Element backing. The backing must live at least as
    ///                     long as the element.
    ///
    Element(T& kBacking) : Element(kBacking, nullptr)
    {
    }

    ///
    /// @brief Constructor for a thread-safe element.
    ///
    /// @warning Because errors cannot be surfaced from Element::read() and
    /// Element::write(), the lock implementation must be errorless, i.e., the
    /// acquire() and release() methods of the provided lock must always return
    /// SUCCESS. When asserts are enabled, Element::read() and Element::write()
    /// will assert that this is the case.
    ///
    /// @remark The element backing should be inaccessible to anything which is
    /// not the element. The backing must live at least as long as the element
    /// it backs.
    ///
    /// @param[in] backing  Element backing. The backing must live at least as
    ///                     long as the element.
    /// @param[in] mLock    Lock to be acquired and released on every element
    ///                     access.
    ///
    Element(T& kBacking, ILock* const kLock) : mBacking(kBacking), mLock(kLock)
    {
    }

    ///
    /// @brief Sets the element value.
    ///
    /// @warning If the element uses a lock and acquiring the lock fails, the
    /// program will halt if asserts are enabled. If asserts are disabled, this
    /// may create a race condition.
    ///
    /// @param[in] kVal  Write value.
    ///
    void write(const T kVal) const
    {
        if (mLock != nullptr)
        {
            // Acquire element lock.
            const Result res = mLock->acquire();
            (void) res;
            SF_ASSERT(res == SUCCESS);
        }

        mBacking = kVal;

        if (mLock != nullptr)
        {
            // Release element lock.
            const Result res = mLock->release();
            (void) res;
            SF_ASSERT(res == SUCCESS);
        }
    }

    ///
    /// @brief Gets the element value.
    ///
    /// @warning If the element uses a lock and acquiring the lock fails, the
    /// program will halt if asserts are enabled. If asserts are disabled, this
    /// may create a race condition.
    ///
    /// @return Read value.
    ///
    T read() const
    {
        if (mLock != nullptr)
        {
            // Acquire element lock.
            const Result res = mLock->acquire();
            (void) res;
            SF_ASSERT(res == SUCCESS);
        }

        const T val = mBacking;

        if (mLock != nullptr)
        {
            // Release element lock.
            const Result res = mLock->release();
            (void) res;
            SF_ASSERT(res == SUCCESS);
        }

        return val;
    }

    ///
    /// @see IElement::type
    ///
    ElementType type() const final override;

    ///
    /// @see IElement::addr
    ///
    const void* addr() const final override
    {
        return &mBacking;
    }

    ///
    /// @see IElement::size
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

    ///
    /// @brief Element lock, or null if none.
    ///
    ILock* const mLock;
};

#endif
