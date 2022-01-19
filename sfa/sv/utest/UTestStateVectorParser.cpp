#include <sstream>

#include "sfa/sv/StateVectorParser.hpp"
#include "UTest.hpp"

TEST_GROUP(StateVectorParser)
{
};

TEST(StateVectorParser, SimpleConfig)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n"
        "[REGION/Bar]\n"
        "F32 qux\n");

    // Check that config is parsed successfully.
    std::shared_ptr<StateVectorParser::Config> config = nullptr;
    CHECK_SUCCESS(StateVectorParser::parse(ss, config, nullptr));

    // Check element names and pointers in returned config.
    const StateVector::Config& svConfig = config->get();
    STRCMP_EQUAL(svConfig.elems[0].name, "foo");
    STRCMP_EQUAL(svConfig.elems[1].name, "bar");
    STRCMP_EQUAL(svConfig.elems[2].name, "baz");
    STRCMP_EQUAL(svConfig.elems[3].name, "qux");
    POINTERS_EQUAL(nullptr, svConfig.elems[4].name);

    // Check element lookups.
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(svConfig, sv));
    Element<I32>* foo = nullptr;
    CHECK_SUCCESS(sv.getElement("foo", foo));
    Element<F64>* bar = nullptr;
    CHECK_SUCCESS(sv.getElement("bar", bar));
    Element<bool>* baz = nullptr;
    CHECK_SUCCESS(sv.getElement("baz", baz));
    Element<F32>* qux = nullptr;
    CHECK_SUCCESS(sv.getElement("qux", qux));

    // Check that element values are initially zero.
    CHECK_EQUAL(0, foo->read());
    CHECK_EQUAL(0.0, bar->read());
    CHECK_EQUAL(false, baz->read());
    CHECK_EQUAL(0.0f, qux->read());

    // Check that the elements can be written and read.
    foo->write(1);
    CHECK_EQUAL(1, foo->read());
    bar->write(1.0);
    CHECK_EQUAL(1.0, bar->read());
    baz->write(true);
    CHECK_EQUAL(true, baz->read());
    qux->write(1.0f);
    CHECK_EQUAL(1.0f, qux->read());

    // Check region lookups.
    Region* regionFoo = nullptr;
    CHECK_SUCCESS(sv.getRegion("Foo", regionFoo));
    Region* regionBar = nullptr;
    CHECK_SUCCESS(sv.getRegion("Bar", regionBar));

    // Check that regions and elements are backed by the same storage.
    #pragma pack(push, 1)
    struct
    {
        struct
        {
            I32 foo;
            F64 bar;
            bool baz;
        } regionFoo;
        struct
        {
            F32 qux;
        } regionBar;
    } vec = {};
    #pragma pack(pop)
    CHECK_SUCCESS(regionFoo->read(&vec.regionFoo, sizeof(vec.regionFoo)));
    CHECK_SUCCESS(regionBar->read(&vec.regionBar, sizeof(vec.regionBar)));
    CHECK_EQUAL(1, vec.regionFoo.foo);
    CHECK_EQUAL(1.0, vec.regionFoo.bar);
    CHECK_EQUAL(true, vec.regionFoo.baz);
    CHECK_EQUAL(1.0f, vec.regionBar.qux);
}

TEST(StateVectorParser, SkipRegion)
{
    // Parse config, but only region `Foo`.
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "[REGION/Bar]\n"
        "bool baz\n"
        "F32 qux\n");
    std::shared_ptr<StateVectorParser::Config> config = nullptr;
    CHECK_SUCCESS(StateVectorParser::parse(ss, config, nullptr, {"Foo"}));

    // Create state vector.
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(config->get(), sv));

    // Getting region `Foo` succeeds.
    Region* region = nullptr;
    CHECK_SUCCESS(sv.getRegion("Foo", region));

    // Getting elements in region `Foo` succeeds.
    Element<I32>* foo = nullptr;
    CHECK_SUCCESS(sv.getElement("foo", foo));
    CHECK_TRUE(foo != nullptr);
    Element<F64>* bar = nullptr;
    CHECK_SUCCESS(sv.getElement("bar", bar));
    CHECK_TRUE(bar != nullptr);

    // Getting region `Bar` fails.
    CHECK_EQUAL(E_KEY, sv.getRegion("Bar", region));

    // Getting elements in region `Bar` fails.
    Element<bool>* baz = nullptr;
    CHECK_EQUAL(E_KEY, sv.getElement("baz", baz));
    POINTERS_EQUAL(nullptr, baz);
    Element<F32>* qux = nullptr;
    CHECK_EQUAL(E_KEY, sv.getElement("qux", qux));
    POINTERS_EQUAL(nullptr, qux);
}
