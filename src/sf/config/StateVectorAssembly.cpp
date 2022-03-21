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
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

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

    return StateVectorAssembly::compile(ifs, kAsm, kErr);
}

Result StateVectorAssembly::compile(std::istream& kIs,
                                    Ref<const StateVectorAssembly>& kAsm,
                                    ErrorInfo* const kErr)
{
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
            if (elems[i]->tokName.str == elems[j]->tokName.str)
            {
                std::stringstream ss;
                ss << "reuse of element name `" << elems[j]->tokName.str
                   << "` (previously used on line "
                   << elems[i]->tokName.lineNum << ")";
                ConfigUtil::setError(
                    kErr, elems[j]->tokName, gErrText, ss.str());
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
            ConfigUtil::setError(kErr, region.tokName, gErrText,
                                 "region is empty");
            return E_SVA_RGN_EMPTY;
        }

        // Add elements into element count.
        elemCnt += region.elems.size();

        // Add element sizes into byte count.
        for (const StateVectorParse::ElementParse& elem : region.elems)
        {
            auto typeInfoIt = TypeInfo::fromName.find(elem.tokType.str);
            if (typeInfoIt == TypeInfo::fromName.end())
            {
                // Unknown element type.
                ConfigUtil::setError(kErr, elem.tokType, gErrText,
                                     "unknown type");
                return E_SVA_ELEM_TYPE;
            }
            const TypeInfo& typeInfo = (*typeInfoIt).second;
            svSizeBytes += typeInfo.sizeBytes;
        }
    }

    // Allocate array for element configs.
    StateVector::ElementConfig* const elemConfigs =
        new StateVector::ElementConfig[elemCnt + 1];
    elemConfigs[elemCnt] = {nullptr, nullptr}; // Null terminator

    // Allocate array for region configs.
    StateVector::RegionConfig* const regionConfigs =
        new StateVector::RegionConfig[regionCnt + 1];
    regionConfigs[regionCnt] = {nullptr, nullptr}; // Null terminator

    // Allocate backing storage for state vector and zero it out.
    char* const svBacking = new char[svSizeBytes];
    std::memset(svBacking, 0, svSizeBytes);

    // Allocate element and region objects and put them into the arrays we just
    // allocated.
    char* bumpPtr = svBacking;
    U32 elemIdx = 0;
    for (U32 regionIdx = 0; regionIdx < kParse->regions.size(); ++regionIdx)
    {
        const StateVectorParse::RegionParse& regionParse =
            kParse->regions[regionIdx];

        // Save a copy of the bump pointer, which right now points to the start
        // of the region.
        char* const regionPtr = bumpPtr;

        // Allocate elements and populate element config array.
        for (const StateVectorParse::ElementParse& elemParse :
             regionParse.elems)
        {
            StateVectorAssembly::allocateElement(elemParse,
                                                 elemConfigs[elemIdx],
                                                 bumpPtr);
            ++elemIdx;
        }

        // Allocate a copy of the region name for the region config object.
        char* regionNameCpy = new char[regionParse.plainName.size() + 1];
        std::strcpy(regionNameCpy, regionParse.plainName.c_str());
        regionConfigs[regionIdx].name = regionNameCpy;

        // Compute the size of the region. Since the `allocateElement` calls
        // above will have bumped the bump pointer to the end of the region,
        // we compute the region size as the difference between the bump pointer
        // and the region pointer we saved at the top of the loop.
        const U64 regionSizeBytes =
            (reinterpret_cast<U64>(bumpPtr) - reinterpret_cast<U64>(regionPtr));

        // Allocate region and put into config array.
        Region* const region = new Region(regionPtr, regionSizeBytes);
        regionConfigs[regionIdx] = {regionNameCpy, region};
    }

    // Config is done- create new state vector with it.
    const StateVector::Config svConfig = {elemConfigs, regionConfigs};
    const Ref<StateVector> sv(new StateVector());
    const Result res = StateVector::create(svConfig, *sv);
    SF_SAFE_ASSERT(res == SUCCESS);

    // Create the final assembly.
    kAsm.reset(new StateVectorAssembly(sv, svConfig, kParse, svBacking));

    return SUCCESS;
}

StateVectorAssembly::~StateVectorAssembly()
{
    // Delete name string and object for each element.
    for (U32 i = 0; mConfig.elems[i].name != nullptr; ++i)
    {
        delete[] mConfig.elems[i].name;
        delete mConfig.elems[i].elem;
    }

    // Delete name string and object for each region.
    for (U32 i = 0; mConfig.regions[i].name != nullptr; ++i)
    {
        delete[] mConfig.regions[i].name;
        delete mConfig.regions[i].region;
    }

    // Delete element config array.
    delete[] mConfig.elems;

    // Delete region config array.
    delete[] mConfig.regions;

    // Delete state vector backing storage.
    delete[] mBacking;
}

Ref<StateVector> StateVectorAssembly::get() const
{
    return mSv;
}

StateVector::Config StateVectorAssembly::config() const
{
    return mConfig;
}

Ref<const StateVectorParse> StateVectorAssembly::parse() const
{
    return mParse;
}

/////////////////////////////////// Private ////////////////////////////////////

void StateVectorAssembly::allocateElement(
    const StateVectorParse::ElementParse& kElem,
    StateVector::ElementConfig& kElemInfo,
    char*& kBumpPtr)
{
    // Allocate a copy of the element name for the element config.
    char* const nameCpy = new char[kElem.tokName.str.size() + 1];
    std::strcpy(nameCpy, kElem.tokName.str.c_str());
    kElemInfo.name = nameCpy;

    // Look up element type info. Assert that this lookup succeeds since the
    // element type was validated earlier.
    SF_ASSERT(kElem.tokType.typeInfo != nullptr);
    const TypeInfo& typeInfo = *kElem.tokType.typeInfo;

    // Allocate element object for element based on its type.
    switch (typeInfo.enumVal)
    {
        case ElementType::INT8:
        {
            I8& backing = *reinterpret_cast<I8*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<I8>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::INT16:
        {
            I16& backing = *reinterpret_cast<I16*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<I16>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::INT32:
        {
            I32& backing = *reinterpret_cast<I32*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<I32>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::INT64:
        {
            I64& backing = *reinterpret_cast<I64*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<I64>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT8:
        {
            U8& backing = *reinterpret_cast<U8*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<U8>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT16:
        {
            U16& backing = *reinterpret_cast<U16*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<U16>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT32:
        {
            U32& backing = *reinterpret_cast<U32*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<U32>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::UINT64:
        {
            U64& backing = *reinterpret_cast<U64*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<U64>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::FLOAT32:
        {
            F32& backing = *reinterpret_cast<F32*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<F32>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::FLOAT64:
        {
            F64& backing = *reinterpret_cast<F64*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<F64>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        case ElementType::BOOL:
        {
            bool& backing = *reinterpret_cast<bool*>(kBumpPtr);
            kElemInfo.elem = static_cast<IElement*>(new Element<bool>(backing));
            kBumpPtr += sizeof(backing);
            break;
        }

        default:
            // Unreachable- would indicate a bug in the element info LUT.
            SF_ASSERT(false);
    }
}

StateVectorAssembly::StateVectorAssembly(
    const Ref<StateVector> kSv,
    const StateVector::Config kConfig,
    const Ref<const StateVectorParse> kParse,
    const char* const kBacking) :
    mSv(kSv), mConfig(kConfig), mParse(kParse), mBacking(kBacking)
{
}
