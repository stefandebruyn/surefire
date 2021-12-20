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

    virtual ElementType getElementType() const = 0;

    virtual U32 getSizeBytes() const = 0;
};

template<typename T>
class Element final : public IElement
{
public:

    Element(T& backing) : mBacking(backing)
    {
    }

    Element(const Element<T>&) = delete;
    Element(Element<T>&&) = delete;
    Element<T>& operator=(const Element<T>&) = delete;
    Element<T>& operator=(Element<T>&&) = delete;

    void write(const T val) const
    {
        mBacking = val;
    }

    T read() const
    {
        return mBacking;
    }

    ElementType getElementType() const final override;

    U32 getSizeBytes() const final override
    {
        return sizeof(T);
    }

private:

    T& mBacking;
};

#endif
