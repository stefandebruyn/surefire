#include <sstream>

#include "sfa/sv/StateVectorParser.hpp"
#include "UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

struct CheckElementInfo final
{
    std::string name;
    ElementType type;
};

struct CheckRegionInfo final
{
    std::string name;
    U32 sizeBytes;
};

static void checkParseStateVectorConfig(
    std::stringstream& kSs,
    const std::vector<CheckElementInfo> kElems,
    const std::vector<CheckRegionInfo> kRegions,
    const std::vector<std::string> kSelectRegions =
        StateVectorParser::ALL_REGIONS)
{
    // Parse state vector config.
    std::shared_ptr<StateVectorParser::Config> config = nullptr;
    CHECK_SUCCESS(
        StateVectorParser::parse(kSs, config, nullptr, kSelectRegions));
    const StateVector::Config& svConfig = config->get();

    // Element names match the parsed config and all element pointers are
    // non-null.
    for (U32 i = 0; i < kElems.size(); ++i)
    {
        STRCMP_EQUAL(kElems[i].name.c_str(), svConfig.elems[i].name);
        CHECK_TRUE(svConfig.elems[i].elem != nullptr);
    }

    // Config element array is null-terminated.
    POINTERS_EQUAL(nullptr, svConfig.elems[kElems.size()].name);
    POINTERS_EQUAL(nullptr, svConfig.elems[kElems.size()].elem);

    // Region names match the parsed config and all region pointers are
    // non-null.
    for (U32 i = 0; i < kRegions.size(); ++i)
    {
        STRCMP_EQUAL(kRegions[i].name.c_str(), svConfig.regions[i].name);
        CHECK_TRUE(svConfig.regions[i].region != nullptr);
    }

    // Config region array is null-terminated.
    POINTERS_EQUAL(nullptr, svConfig.regions[kRegions.size()].name);
    POINTERS_EQUAL(nullptr, svConfig.regions[kRegions.size()].region);

    // Create state vector.
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(svConfig, sv));

    // All elements can be looked up and have initial value zero.
    for (const CheckElementInfo& elemInfo : kElems)
    {
        switch (elemInfo.type)
        {
            case INT8:
            {
                Element<I8>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case INT16:
            {
                Element<I16>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case INT32:
            {
                Element<I32>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case INT64:
            {
                Element<I64>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case UINT8:
            {
                Element<U8>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case UINT16:
            {
                Element<U16>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case UINT32:
            {
                Element<U32>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case UINT64:
            {
                Element<U64>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case FLOAT32:
            {
                Element<F32>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case FLOAT64:
            {
                Element<F64>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case BOOL:
            {
                Element<bool>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            default:
                FAIL("Unreachable");
        }
    }

    // All regions can be looked up and have the correct size.
    for (const CheckRegionInfo& regionInfo : kRegions)
    {
        Region* region = nullptr;
        CHECK_SUCCESS(sv.getRegion(regionInfo.name.c_str(), region));
        CHECK_EQUAL(regionInfo.sizeBytes, region->size());
    }

    // State vector backing memory is contiguous and consistent with the
    // configured order of regions and elements.
    const StateVector::RegionConfig* regionConfig = &svConfig.regions[0];
    const char* svBumpPtr =
        static_cast<const char*>(regionConfig->region->addr());
    for (const StateVector::ElementConfig* elemConfig = &svConfig.elems[0];
         elemConfig->name != nullptr;
         ++elemConfig)
    {
        // Element is at the bump pointer.
        POINTERS_EQUAL(svBumpPtr, elemConfig->elem->addr());
        svBumpPtr += elemConfig->elem->size();

        // Move to the next region when the bump pointer hits the end of the
        // current region.
        const char* const regionEnd =
            (static_cast<const char*>(regionConfig->region->addr())
             + regionConfig->region->size());
        if (svBumpPtr == regionEnd)
        {
            ++regionConfig;
            if (regionConfig->name != nullptr)
            {
                // Next region is not the last region- it starts at the address
                // the previous region ended.
                POINTERS_EQUAL(regionEnd, regionConfig->region->addr());
            }
            else
            {
                // No next region- current element is the last element in the
                // state vector.
                const StateVector::ElementConfig* const nextElemConfig =
                    (elemConfig + 1);
                POINTERS_EQUAL(nullptr, nextElemConfig->name);
            }
        }
    }
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateVectorParserParse)
{
};

TEST(StateVectorParserParse, SimpleConfig)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n"
        "[REGION/Bar]\n"
        "F32 qux\n");

    checkParseStateVectorConfig(
        ss,
        {
            {"foo", INT32}, {"bar", FLOAT64}, {"baz", BOOL},
            {"qux", FLOAT32}
        },
        {
            {"Foo", 13}, {"Bar", 4}
        });
}

TEST(StateVectorParserParse, AllElementTypes)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "F32 a\n"
        "U8 b\n"
        "I8 c\n"
        "I64 d\n"
        "[REGION/Bar]\n"
        "U16 e\n"
        "bool f\n"
        "I16 g\n"
        "[REGION/Baz]\n"
        "F64 h\n"
        "U64 i\n"
        "I32 j\n"
        "U32 k\n");

    checkParseStateVectorConfig(
        ss,
        {
            {"a", FLOAT32}, {"b", UINT8}, {"c", INT8}, {"d", INT64},
            {"e", UINT16}, {"f", BOOL}, {"g", INT16},
            {"h", FLOAT64}, {"i", UINT64}, {"j", INT32}, {"k", UINT32}
        },
        {
            {"Foo", 14},
            {"Bar", 5},
            {"Baz", 24}
        });
}

TEST(StateVectorParserParse, OneRegionOneElement)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n");

    checkParseStateVectorConfig(ss, {{"foo", INT32}}, {{"Foo", 4}});
}

TEST(StateVectorParserParse, OneLargeRegion)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 a0\n"
        "I32 a1\n"
        "I32 a2\n"
        "I32 a3\n"
        "I32 a4\n"
        "I32 a5\n"
        "I32 a6\n"
        "I32 a7\n"
        "F64 b0\n"
        "F64 b1\n"
        "F64 b2\n"
        "F64 b3\n"
        "F64 b4\n"
        "F64 b5\n"
        "F64 b6\n"
        "F64 b7\n"
        "bool c0\n"
        "bool c1\n"
        "bool c2\n"
        "bool c3\n"
        "bool c4\n"
        "bool c5\n"
        "bool c6\n"
        "bool c7\n"
        "U16 d0\n"
        "U16 d1\n"
        "U16 d2\n"
        "U16 d3\n"
        "U16 d4\n"
        "U16 d5\n"
        "U16 d6\n"
        "U16 d7\n"
        "F32 e0\n"
        "F32 e1\n"
        "F32 e2\n"
        "F32 e3\n"
        "F32 e4\n"
        "F32 e5\n"
        "F32 e6\n"
        "F32 e7\n");

    checkParseStateVectorConfig(
        ss,
        {
            {"a0", INT32}, {"a1", INT32}, {"a2", INT32}, {"a3", INT32},
            {"a4", INT32}, {"a5", INT32}, {"a6", INT32}, {"a7", INT32},
            {"b0", FLOAT64}, {"b1", FLOAT64}, {"b2", FLOAT64}, {"b3", FLOAT64},
            {"b4", FLOAT64}, {"b5", FLOAT64}, {"b6", FLOAT64}, {"b7", FLOAT64},
            {"c0", BOOL}, {"c1", BOOL}, {"c2", BOOL}, {"c3", BOOL},
            {"c4", BOOL}, {"c5", BOOL}, {"c6", BOOL}, {"c7", BOOL},
            {"d0", UINT16}, {"d1", UINT16}, {"d2", UINT16}, {"d3", UINT16},
            {"d4", UINT16}, {"d5", UINT16}, {"d6", UINT16}, {"d7", UINT16},
            {"e0", FLOAT32}, {"e1", FLOAT32}, {"e2", FLOAT32}, {"e3", FLOAT32},
            {"e4", FLOAT32}, {"e5", FLOAT32}, {"e6", FLOAT32}, {"e7", FLOAT32}
        },
        {
            {"Foo", 152}
        });
}

TEST(StateVectorParserParse, NoNewlines)
{
    std::stringstream ss(
        "[REGION/Foo] I32 foo F64 bar [REGION/Bar] bool baz F32 qux");

    checkParseStateVectorConfig(
        ss,
        {
            {"foo", INT32}, {"bar", FLOAT64},
            {"baz", BOOL}, {"qux", FLOAT32}
        },
        {
            {"Foo", 12},
            {"Bar", 5}
        });
}

TEST(StateVectorParserParse, SelectSpecificRegions)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 a\n"
        "F64 b\n"
        "[REGION/Bar]\n"
        "bool c\n"
        "U16 d\n"
        "[REGION/Baz]\n"
        "F32 e\n"
        "U8 f\n"
        "[REGION/Qux]\n"
        "U32 g\n"
        "I64 h\n");

    checkParseStateVectorConfig(
        ss,
        {
            {"c", BOOL}, {"d", UINT16},
            {"g", UINT32}, {"h", INT64}
        },
        {
            {"Bar", 3},
            {"Qux", 12}
        },
        {"Bar", "Qux"});
}
