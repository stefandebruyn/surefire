#ifndef SFA_ELEMENT_HPP
#define SFA_ELEMENT_HPP

#include "sfa/BasicTypes.hpp"

enum ElementType : U32
{
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT32,
    FLOAT64,
    BOOL
};

class IElement
{
public:

    virtual ~IElement() = default;

    virtual ElementType type() const = 0;

    virtual const void* addr() const = 0;

    virtual U32 size() const = 0;
};

template<typename T>
class Element final : public IElement
{
public:

    Element(T& backing) : mBacking(backing)
    {
    }

    void write(const T val) const
    {
        mBacking = val;
    }

    T read() const
    {
        return mBacking;
    }

    ElementType type() const final override;

    const void* addr() const final override
    {
        return &mBacking;
    }

    U32 size() const final override
    {
        return sizeof(T);
    }

    Element(const Element<T>&) = delete;
    Element(Element<T>&&) = delete;
    Element<T>& operator=(const Element<T>&) = delete;
    Element<T>& operator=(Element<T>&&) = delete;

private:

    T& mBacking;
};

#endif
