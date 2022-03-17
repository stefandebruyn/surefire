#include "sfa/sup/StateVectorCompiler.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

struct ElementInfo final
{
    std::string name;
    ElementType type;
};

struct RegionInfo final
{
    std::string name;
    U32 sizeBytes;
};

static void checkStateVectorConfig(const std::vector<Token>& kToks,
                                   const std::vector<ElementInfo> kElems,
                                   const std::vector<RegionInfo> kRegions)
{
    // Parse and compile state vector config.
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(kToks, parse, nullptr));
    std::shared_ptr<StateVectorCompiler::Assembly> assembly;
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

static void checkCompileError(const std::vector<Token>& kToks,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Parse state vector config.
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(kToks, parse, nullptr));

    // Got expected return code from compiler.
    std::shared_ptr<StateVectorCompiler::Assembly> assembly;
    ConfigErrorInfo err;
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
            {"foo", INT32}, {"bar", FLOAT64}, {"baz", BOOL}, {"qux", FLOAT32}
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
            {"a", FLOAT32}, {"b", UINT8}, {"c", INT8}, {"d", INT64},
            {"e", UINT16}, {"f", BOOL}, {"g", INT16}, {"h", FLOAT64},
            {"i", UINT64}, {"j", INT32}, {"k", UINT32}
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

TEST(StateVectorCompiler, NewlineAgnostic)
{
    TOKENIZE(
        "[Foo] I32 foo F64 bar [Bar] bool baz F32 qux");
    checkStateVectorConfig(
        toks,
        {
            {"foo", INT32}, {"bar", FLOAT64},
            {"baz", BOOL}, {"qux", FLOAT32}
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
