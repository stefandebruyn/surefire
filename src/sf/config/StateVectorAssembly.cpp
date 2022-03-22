#include <fstream>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

extern const char* const gErrText = "state vector config error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateVectorAssembly::compile(const String kFilePath,
                                    Ref<const StateVectorAssembly>& kAsm,
                                    ErrorInfo* const kErr)
{
    // Open file input stream.
    std::ifstream ifs(kFilePath);
    if (!ifs.is_open())
    {
        if (kErr != nullptr)
        {
            kErr->text = "error";
            kErr->subtext = "failed to open file `" + kFilePath + "`";
        }
        return E_SMA_FILE;
    }

    // Set the error info file path for error messages generated further into
    // compilation.
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

    // Send input stream into the next compilation phase.
    return StateVectorAssembly::compile(ifs, kAsm, kErr);
}

Result StateVectorAssembly::compile(std::istream& kIs,
                                    Ref<const StateVectorAssembly>& kAsm,
                                    ErrorInfo* const kErr)
{
    // Tokenize the input stream.
    Vec<Token> toks;
    Result res = Tokenizer::tokenize(kIs, toks, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }
        return res;
    }

    // Parse the state machine config.
    Ref<const StateVectorParse> parse;
    res = StateVectorParse::parse(toks, parse, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
        }
        return res;
    }

    // Send state machine config into the next compilation phase.
    return StateVectorAssembly::compile(parse, kAsm, kErr);
}

Result StateVectorAssembly::compile(const Ref<const StateVectorParse> kParse,
                                    Ref<const StateVectorAssembly>& kAsm,
                                    ErrorInfo* const kErr)
{
    // Check that state vector parse is non-null.
    if (kParse == nullptr)
    {
        return E_SVA_NULL;
    }

    // Check that region names are unique. While we do this, collect all the
    // elements in a list so that we can check element name uniqueness.
    Vec<const StateVectorParse::ElementParse*> elems;
    for (const StateVectorParse::RegionParse& i : kParse->regions)
    {
        for (const StateVectorParse::RegionParse& j : kParse->regions)
        {
            if ((&i != &j) && (i.plainName == j.plainName))
            {
                std::stringstream ss;
                ss << "reuse of region name `" << j.plainName
                   << "` (previously used on line " << i.tokName.lineNum << ")";
                ConfigUtil::setError(kErr, j.tokName, gErrText, ss.str());
                return E_SVA_RGN_DUPE;
            }
        }

        for (const StateVectorParse::ElementParse& elem : i.elems)
        {
            elems.push_back(&elem);
        }
    }

    // Check that element names are unique.
    for (U32 i = 0; i < elems.size(); ++i)
    {
        for (U32 j = (i + 1); j < elems.size(); ++j)
        {
            SF_SAFE_ASSERT(elems[i] != nullptr);
            SF_SAFE_ASSERT(elems[j] != nullptr);
            if (elems[i]->tokName.str == elems[j]->tokName.str)
            {
                std::stringstream ss;
                ss << "reuse of element name `" << elems[j]->tokName.str
                   << "` (previously used on line "
                   << elems[i]->tokName.lineNum << ")";
                ConfigUtil::setError(kErr,
                                     elems[j]->tokName,
                                     gErrText,
                                     ss.str());
                return E_SVA_ELEM_DUPE;
            }
        }
    }

    // Count the number of elements, regions, and bytes in the state vector.
    U32 elemCnt = 0;
    const U32 regionCnt = kParse->regions.size();
    U32 svSizeBytes = 0;
    for (const StateVectorParse::RegionParse& region : kParse->regions)
    {
        // Check that region contains at least 1 element.
        if (region.elems.size() == 0)
        {
            ConfigUtil::setError(kErr,
                                 region.tokName,
                                 gErrText,
                                 "region is empty");
            return E_SVA_RGN_EMPTY;
        }

        // Add elements into element count.
        elemCnt += region.elems.size();

        // Add element sizes into byte count.
        for (const StateVectorParse::ElementParse& elem : region.elems)
        {
            // Look up element type info.
            auto typeInfoIt = TypeInfo::fromName.find(elem.tokType.str);
            if (typeInfoIt == TypeInfo::fromName.end())
            {
                // Unknown element type.
                ConfigUtil::setError(kErr,
                                     elem.tokType,
                                     gErrText,
                                     "unknown type");
                return E_SVA_ELEM_TYPE;
            }

            const TypeInfo& typeInfo = (*typeInfoIt).second;
            svSizeBytes += typeInfo.sizeBytes;
        }
    }

    // Initialize a blank workspace for the compilation.
    StateVectorAssembly::Workspace ws = {};

    // Put the state vector parse in the workspace so that it can be recalled
    // later.
    ws.svParse = kParse;

    // Allocate array for element configs.
    ws.elemConfigs.reset(new Vec<StateVector::ElementConfig>(elemCnt + 1));

    // Set null terminator for element config array required by state vector.
    (*ws.elemConfigs)[elemCnt] = {nullptr, nullptr};

    // Allocate array for region configs.
    ws.regionConfigs.reset(new Vec<StateVector::RegionConfig>(regionCnt + 1));

    // Set null terminator for region config array required by state vector.
    (*ws.regionConfigs)[regionCnt] = {nullptr, nullptr};

    // Allocate backing storage for state vector. This memory will be
    // automatically zeroed out by the vector implementation, ensuring that
    // state vector elements default to zero. (A vector is used instead of a
    // native array since C++11 shared pointers are not specialized for native
    // arrays.)
    ws.svBacking.reset(new Vec<U8>(svSizeBytes));

    // Now to initialize the members of the element and region config arrays.
    // This pointer stores the address of the next element's backing storage
    // and will be bumped along as elements are allocated.
    U8* bumpPtr = ws.svBacking->data();

    // Index of the current element within the entire state vector.
    U32 elemIdx = 0;

    for (U32 regionIdx = 0; regionIdx < kParse->regions.size(); ++regionIdx)
    {
        // Get region parse.
        const StateVectorParse::RegionParse& regionParse =
            kParse->regions[regionIdx];

        // Save a copy of the bump pointer, which right now points to the start
        // of the region.
        U8* const regionPtr = bumpPtr;

        // Allocate elements in region and populate element config array.
        for (const StateVectorParse::ElementParse& elemParse :
             regionParse.elems)
        {
            const Result res = StateVectorAssembly::allocateElement(
                elemParse,
                ws,
                (*ws.elemConfigs)[elemIdx],
                bumpPtr);
            if (res != SUCCESS)
            {
                return res;
            }

            ++elemIdx;
        }

        // Allocate a copy of the region name and put the raw pointer in the
        // region config.
        const Ref<String> regionNameCpy(new String(regionParse.plainName));
        (*ws.regionConfigs)[regionIdx].name = regionNameCpy->c_str();

        // Add region name to workspace.
        ws.configStrings.push_back(regionNameCpy);

        // Compute the size of the region. Since the element allocations will
        // have bumped the bump pointer to the end of the region, we compute the
        // region size as the difference between the bump pointer and the region
        // pointer we saved at the top of the loop.
        const U64 regionSizeBytes = (reinterpret_cast<U64>(bumpPtr)
                                     - reinterpret_cast<U64>(regionPtr));

        // Allocate region object, add it to the workspace, and put raw pointers
        // to the region name and object in the region config array.
        const Ref<Region> region(new Region(regionPtr, regionSizeBytes));
        ws.regions.push_back(region);
        (*ws.regionConfigs)[regionIdx] = {regionNameCpy->c_str(), region.get()};
    }

    // Config is done- create new state vector with it. Assert that creation
    // succeeds since the state vector config is known correct at this point.
    ws.svConfig = {ws.elemConfigs->data(), ws.regionConfigs->data()};
    ws.sv.reset(new StateVector());
    const Result res = StateVector::create(ws.svConfig, *ws.sv);
    SF_SAFE_ASSERT(res == SUCCESS);

    // Create the final assembly.
    kAsm.reset(new StateVectorAssembly(ws));

    return SUCCESS;
}

Ref<StateVector> StateVectorAssembly::get() const
{
    return mWs.sv;
}

StateVector::Config StateVectorAssembly::config() const
{
    return mWs.svConfig;
}

Ref<const StateVectorParse> StateVectorAssembly::parse() const
{
    return mWs.svParse;
}

/////////////////////////////////// Private ////////////////////////////////////

Result StateVectorAssembly::allocateElement(
    const StateVectorParse::ElementParse& kElem,
    StateVectorAssembly::Workspace& kWs,
    StateVector::ElementConfig& kElemConfig,
    U8*& kBumpPtr)
{
    SF_SAFE_ASSERT(kBumpPtr != nullptr);

    // Allocate a copy of the element name, add it to the workspace, and put the
    // raw pointer in the element config.
    const Ref<String> elemNameCpy(new String(kElem.tokName.str));
    kWs.configStrings.push_back(elemNameCpy);
    kElemConfig.name = elemNameCpy->c_str();

    // Get element type info.
    SF_SAFE_ASSERT(kElem.tokType.typeInfo != nullptr);
    const TypeInfo& typeInfo = *kElem.tokType.typeInfo;

    // Allocate element object for element based on its type and bump the bump
    // pointer by the element's size.
    Ref<IElement> elemObj;
    switch (typeInfo.enumVal)
    {
        case ElementType::INT8:
        {
            I8& backing = *reinterpret_cast<I8*>(kBumpPtr);
            elemObj.reset(new Element<I8>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::INT16:
        {
            I16& backing = *reinterpret_cast<I16*>(kBumpPtr);
            elemObj.reset(new Element<I16>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::INT32:
        {
            I32& backing = *reinterpret_cast<I32*>(kBumpPtr);
            elemObj.reset(new Element<I32>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::INT64:
        {
            I64& backing = *reinterpret_cast<I64*>(kBumpPtr);
            elemObj.reset(new Element<I64>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT8:
        {
            U8& backing = *reinterpret_cast<U8*>(kBumpPtr);
            elemObj.reset(new Element<U8>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT16:
        {
            U16& backing = *reinterpret_cast<U16*>(kBumpPtr);
            elemObj.reset(new Element<U16>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT32:
        {
            U32& backing = *reinterpret_cast<U32*>(kBumpPtr);
            elemObj.reset(new Element<U32>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT64:
        {
            U64& backing = *reinterpret_cast<U64*>(kBumpPtr);
            elemObj.reset(new Element<U64>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::FLOAT32:
        {
            F32& backing = *reinterpret_cast<F32*>(kBumpPtr);
            elemObj.reset(new Element<F32>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::FLOAT64:
        {
            F64& backing = *reinterpret_cast<F64*>(kBumpPtr);
            elemObj.reset(new Element<F64>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::BOOL:
        {
            bool& backing = *reinterpret_cast<bool*>(kBumpPtr);
            elemObj.reset(new Element<bool>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        default:
            // Unreachable.
            SF_SAFE_ASSERT(false);
    }

    // Add allocated element to workspace and put the raw pointer in the element
    // config.
    kWs.elems.push_back(elemObj);
    kElemConfig.elem = elemObj.get();

    return SUCCESS;
}

StateVectorAssembly::StateVectorAssembly(
    const StateVectorAssembly::Workspace& kWs) : mWs(kWs)
{
}
