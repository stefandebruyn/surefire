#ifndef SF_ELEMENT_HPP
#define SF_ELEMENT_HPP

#include "sf/core/BasicTypes.hpp"

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
