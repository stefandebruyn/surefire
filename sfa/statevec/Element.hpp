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

    Element(T& backing) : mBacking(&backing)
    {
    }

    Element() : mBacking(nullptr)
    {
    }

    void write(const T val) const
    {
        if (mBacking != nullptr)
        {
            *mBacking = val;
        }
    }

    T read() const
    {
        T ret = 0;
        if (mBacking != nullptr)
        {
            ret = *mBacking;
        }
        return ret;
    }

    ElementType getElementType() const final override;

    U32 getSizeBytes() const final override
    {
        return sizeof(T);
    }

private:

    T* mBacking;
};

#endif
