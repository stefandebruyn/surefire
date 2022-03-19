#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

struct ElementInfo final
{
    String name;
    ElementType type;
};

struct RegionInfo final
{
    String name;
    U32 sizeBytes;
};

static void checkStateVectorConfig(const Vec<Token>& kToks,
                                   const Vec<ElementInfo> kElems,
                                   const Vec<RegionInfo> kRegions)
{
    // Parse and compile state vector config.
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(kToks, parse, nullptr));
    Ref<const StateVectorCompiler::Assembly> assembly;
    CHECK_SUCCESS(StateVectorCompiler::compile(parse, assembly, nullptr));
    const StateVector::Config& svConfig = assembly->getConfig();

    // Element names match the parsed config and all element pointers are
    // non-null.
    for (U32 i = 0; i < kElems.size(); ++i)
    {
        CHECK_TRUE(svConfig.elems[i].name != nullptr);
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
    for (const ElementInfo& elemInfo : kElems)
    {
        switch (elemInfo.type)
        {
            case ElementType::INT8:
            {
                Element<I8>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::INT16:
            {
                Element<I16>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::INT32:
            {
                Element<I32>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::INT64:
            {
                Element<I64>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::UINT8:
            {
                Element<U8>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::UINT16:
            {
                Element<U16>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::UINT32:
            {
                Element<U32>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::UINT64:
            {
                Element<U64>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::FLOAT32:
            {
                Element<F32>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::FLOAT64:
            {
                Element<F64>* elem = nullptr;
                CHECK_SUCCESS(sv.getElement(elemInfo.name.c_str(), elem));
                CHECK_EQUAL(0, elem->read());
                break;
            }

            case ElementType::BOOL:
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
    for (const RegionInfo& regionInfo : kRegions)
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

static void checkCompileError(const Vec<Token>& kToks,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Parse state vector config.
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(kToks, parse, nullptr));

    // Got expected return code from compiler.
    Ref<const StateVectorCompiler::Assembly> assembly;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateVectorCompiler::compile(parse, assembly, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Assembly pointer was not populated.
    CHECK_TRUE(assembly == nullptr);
}

///////////////////////////////// Usage Tests //////////////////////////////////

TEST_GROUP(StateVectorCompiler)
{
};

TEST(StateVectorCompiler, OneElement)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n");
    checkStateVectorConfig(
        toks,
        {
            {"foo", INT32}
        },
        {
            {"Foo", 4}
        });
}

TEST(StateVectorCompiler, SimpleConfig)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n"
        "[Bar]\n"
        "F32 qux\n");
    checkStateVectorConfig(
        toks,
        {
            {"foo", ElementType::INT32},
            {"bar", ElementType::FLOAT64},
            {"baz", ElementType::BOOL},
            {"qux", ElementType::FLOAT32}
        },
        {
            {"Foo", 13}, {"Bar", 4}
        });
}

TEST(StateVectorCompiler, AllElementTypes)
{
    TOKENIZE(
        "[Foo]\n"
        "F32 a\n"
        "U8 b\n"
        "I8 c\n"
        "I64 d\n"
        "[Bar]\n"
        "U16 e\n"
        "bool f\n"
        "I16 g\n"
        "[Baz]\n"
        "F64 h\n"
        "U64 i\n"
        "I32 j\n"
        "U32 k\n");
    checkStateVectorConfig(
        toks,
        {
            {"a", ElementType::FLOAT32},
            {"b", ElementType::UINT8},
            {"c", ElementType::INT8},
            {"d", ElementType::INT64},
            {"e", ElementType::UINT16},
            {"f", ElementType::BOOL},
            {"g", ElementType::INT16},
            {"h", ElementType::FLOAT64},
            {"i", ElementType::UINT64},
            {"j", ElementType::INT32},
            {"k", ElementType::UINT32}
        },
        {
            {"Foo", 14}, {"Bar", 5}, {"Baz", 24}
        });
}

TEST(StateVectorCompiler, OneLargeRegion)
{
    TOKENIZE(
        "[Foo]\n"
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
    checkStateVectorConfig(
        toks,
        {
            {"a0", ElementType::INT32},
            {"a1", ElementType::INT32},
            {"a2", ElementType::INT32},
            {"a3", ElementType::INT32},
            {"a4", ElementType::INT32},
            {"a5", ElementType::INT32},
            {"a6", ElementType::INT32},
            {"a7", ElementType::INT32},
            {"b0", ElementType::FLOAT64},
            {"b1", ElementType::FLOAT64},
            {"b2", ElementType::FLOAT64},
            {"b3", ElementType::FLOAT64},
            {"b4", ElementType::FLOAT64},
            {"b5", ElementType::FLOAT64},
            {"b6", ElementType::FLOAT64},
            {"b7", ElementType::FLOAT64},
            {"c0", ElementType::BOOL},
            {"c1", ElementType::BOOL},
            {"c2", ElementType::BOOL},
            {"c3", ElementType::BOOL},
            {"c4", ElementType::BOOL},
            {"c5", ElementType::BOOL},
            {"c6", ElementType::BOOL},
            {"c7", ElementType::BOOL},
            {"d0", ElementType::UINT16},
            {"d1", ElementType::UINT16},
            {"d2", ElementType::UINT16},
            {"d3", ElementType::UINT16},
            {"d4", ElementType::UINT16},
            {"d5", ElementType::UINT16},
            {"d6", ElementType::UINT16},
            {"d7", ElementType::UINT16},
            {"e0", ElementType::FLOAT32},
            {"e1", ElementType::FLOAT32},
            {"e2", ElementType::FLOAT32},
            {"e3", ElementType::FLOAT32},
            {"e4", ElementType::FLOAT32},
            {"e5", ElementType::FLOAT32},
            {"e6", ElementType::FLOAT32},
            {"e7", ElementType::FLOAT32}
        },
        {
            {"Foo", 152}
        });
}

TEST(StateVectorCompiler, NewlineAgnostic)
{
    TOKENIZE(
        "[Foo] I32 foo F64 bar [Bar] bool baz F32 qux");
    checkStateVectorConfig(
        toks,
        {
            {"foo", ElementType::INT32},
            {"bar", ElementType::FLOAT64},
            {"baz", ElementType::BOOL},
            {"qux", ElementType::FLOAT32}
        },
        {
            {"Foo", 12},
            {"Bar", 5}
        });
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateVectorCompilerErrors)
{
};

TEST(StateVectorCompilerErrors, DuplicateElementNameSameRegion)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 foo\n");
    checkCompileError(toks, E_SVC_ELEM_DUPE, 3, 5);
}

TEST(StateVectorCompilerErrors, DuplicateElementNameDifferentRegion)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "[Bar]\n"
        "F64 foo\n");
    checkCompileError(toks, E_SVC_ELEM_DUPE, 4, 5);
}

TEST(StateVectorCompilerErrors, DuplicateRegionName)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "[Foo]\n"
        "F64 bar\n");
    checkCompileError(toks, E_SVC_RGN_DUPE, 3, 1);
}

TEST(StateVectorCompilerErrors, EmptyRegion)
{
    TOKENIZE(
        "[Foo]\n");
    checkCompileError(toks, E_SVC_RGN_EMPTY, 1, 1);
}

TEST(StateVectorCompilerErrors, UnknownElementType)
{
    TOKENIZE(
        "[Foo]\n"
        "I33 foo\n");
    checkCompileError(toks, E_SVC_ELEM_TYPE, 2, 1);
}
