#include "sfa/statevec/Element.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/UTest.hpp"

TEST_GROUP(Element)
{
};

TEST(Element, ReadWrite)
{
    I32 backing = 1;
    Element<I32> elem(backing);
    CHECK_EQUAL(1, elem.read());
    elem.write(2);
    CHECK_EQUAL(2, elem.read());
}
