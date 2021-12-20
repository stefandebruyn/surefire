#include <fstream>
#include <cstring>

#include "sfa/statevec/StateVectorParser.hpp"

StateVectorParser::Config::Config(const StateVector::Config kSvConfig,
                                  const char* const kSvBacking) :
    mSvConfig(kSvConfig), mSvBacking(kSvBacking)
{
}

StateVectorParser::Config::~Config()
{
    // Delete name string and object for each element.
    U32 i = 0;
    while (mSvConfig.elems[i].name != nullptr)
    {
        delete[] mSvConfig.elems[i].name;
        delete mSvConfig.elems[i].elem;
        ++i;
    }

    // Delete name string and object for each region.
    i = 0;
    while (mSvConfig.regions[i].name != nullptr)
    {
        delete[] mSvConfig.regions[i].name;
        delete mSvConfig.regions[i].region;
        ++i;
    }

    // Delete element config array.
    delete[] mSvConfig.elems;

    // Delete region config array.
    delete[] mSvConfig.regions;

    // Delete state vector backing storage.
    delete[] mSvBacking;
}

const StateVector::Config& StateVectorParser::Config::get() const
{
    return mSvConfig;
}

const std::regex StateVectorParser::mRegionSectionRegex(
    "REGION/([a-zA-Z][a-zA-Z0-9_]*)");

const std::unordered_map<std::string, U32> StateVectorParser::mElemTypeSize =
{
    {"I8", 1},
    {"I16", 2},
    {"I32", 4},
    {"I64", 8},
    {"U8", 1},
    {"U16", 2},
    {"U32", 4},
    {"U64", 8},
    {"F32", 4},
    {"F64", 8},
    {"bool", 1}
};

Result StateVectorParser::parse(const std::string kFilePath,
                                std::shared_ptr<Config>& kConfig,
                                ConfigInfo* kConfigInfo)
{
    std::ifstream ifs(kFilePath);
    if (ifs.is_open() == false)
    {
        if (kConfigInfo != nullptr)
        {
            kConfigInfo->error.msg = "failed to open file: " + kFilePath;
        }
        return E_OPEN_FILE;
    }

    if (kConfigInfo != nullptr)
    {
        kConfigInfo->filePath = kFilePath;
    }

    return StateVectorParser::parse(ifs, kConfig, kConfigInfo);
}

Result StateVectorParser::parse(std::istream& kIs,
                                std::shared_ptr<Config>& kConfig,
                                ConfigInfo* kConfigInfo)
{
    std::vector<Token> toks;
    Result res = Tokenizer::tokenize(kIs, toks, kConfigInfo);
    if (res != SUCCESS)
    {
        return res;
    }

    return StateVectorParser::parseImpl(toks, kConfig, kConfigInfo);
}

Result StateVectorParser::parseImpl(const std::vector<Token>& kToks,
                                    std::shared_ptr<Config>& kConfig,
                                    ConfigInfo* kConfigInfo)
{
    Parse parse;

    // Process token stream to populate `parse`.
    U32 idx = 0;
    while (idx < kToks.size())
    {
        const Token& tok = kToks[idx];
        switch (tok.type)
        {
            case TOK_SECTION:
            {
                // Consume section token.
                ++idx;

                // Parse section.
                std::smatch match;
                if (std::regex_match(
                    tok.str, match, mRegionSectionRegex) == true)
                {
                    // Parse region section.
                    RegionParse region;
                    region.tokName = tok;
                    region.plainName = match[1].str();
                    Result res = StateVectorParser::parseRegion(
                        kToks, idx, region, kConfigInfo);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                    parse.regions.push_back(region);
                }
                else
                {
                    // Unknown section.
                    if (kConfigInfo != nullptr)
                    {
                        kConfigInfo->error.lineNum = tok.lineNum;
                        kConfigInfo->error.colNum = tok.colNum;
                        kConfigInfo->error.msg = "invalid section name";
                    }
                    return E_PARSE;
                }
                break;
            }

            case TOK_NEWLINE:
                // Consume newline token.
                ++idx;
                break;

            default:
                // Unexpected token.
                if (kConfigInfo != nullptr)
                {
                    kConfigInfo->error.lineNum = tok.lineNum;
                    kConfigInfo->error.colNum = tok.colNum;
                    auto iter = gTokenNames.find(tok.type);
                    if (iter == gTokenNames.end())
                    {
                        // Should be unreachable- indicates that `gTokenNames`
                        // is missing a key.
                        return E_KEY;
                    }
                    const std::string& tokTypeName = (*iter).second;
                    kConfigInfo->error.msg = "unexpected " + tokTypeName;
                }
                return E_PARSE;
        }
    }

    // At this point `parse` contains a potentially valid state vector parsing-
    // now we try compiling it into a `StateVectorParser::Config`.

    // Count the number of elements, regions, and bytes in the state vector.
    U32 elemCnt = 0;
    const U32 regionCnt = parse.regions.size();
    U32 svSizeBytes = 0;
    for (const RegionParse& region : parse.regions)
    {
        elemCnt += region.elems.size();
        for (const ElementParse& elem : region.elems)
        {
            auto iter = mElemTypeSize.find(elem.tokType.str);
            if (iter == mElemTypeSize.end())
            {
                // Unknown element type.
                if (kConfigInfo != nullptr)
                {
                    kConfigInfo->error.lineNum = elem.tokType.lineNum;
                    kConfigInfo->error.colNum = elem.tokType.colNum;
                    kConfigInfo->error.msg =
                        "unknown type `" + elem.tokType.str + "`";
                }
                return E_PARSE;
            }
            svSizeBytes += (*iter).second;
        }
    }

    // Allocate array for element configs.
    StateVector::ElementConfig* elemConfigs =
        new StateVector::ElementConfig[elemCnt + 1];
    elemConfigs[elemCnt] = {nullptr, nullptr}; // Null terminator

    // Allocate array for region configs.
    StateVector::RegionConfig* regionConfigs =
        new StateVector::RegionConfig[regionCnt + 1];
    regionConfigs[regionCnt] = {nullptr, nullptr};

    // Allocate backing storage for state vector and zero it out.
    char* svBacking = new char[svSizeBytes];
    std::memset(svBacking, 0, svSizeBytes);

    // Allocate `Element` and `Region` objects and put them into the arrays
    // we just allocated.
    char* bumpPtr = svBacking;
    U32 elemIdx = 0;
    for (U32 regionIdx = 0; regionIdx < parse.regions.size(); ++regionIdx)
    {
        const RegionParse& regionParse = parse.regions[regionIdx];

        // Save a copy of the bump pointer, which right now points to the start
        // of the region.
        char* const regionPtr = bumpPtr;

        // Allocate elements and populate element config array.
        for (const ElementParse& elemParse : regionParse.elems)
        {
            Result res = StateVectorParser::allocateElement(
                elemParse, elemConfigs[elemIdx], bumpPtr);
            if (res != SUCCESS)
            {
                // Clean up allocations since aborting parse.
                delete[] elemConfigs;
                delete[] regionConfigs;
                delete[] svBacking;
                return res;
            }
            ++elemIdx;
        }

        // Allocate a copy of the region name for the
        // `StateVector::RegionConfig` representing this region.
        char* regionNameCpy = new char[regionParse.plainName.size() + 1];
        std::strcpy(regionNameCpy, regionParse.plainName.c_str());
        regionConfigs[regionIdx].name = regionNameCpy;

        // Compute the size of the region. Since the `allocateElement` calls
        // above will have bumped the bump pointer to the end of the region,
        // we compute the region size as the difference between the bump pointer
        // and the region pointer we saved at the top of the loop.
        const U64 regionSizeBytes = ((U64) bumpPtr - (U64) regionPtr);

        // Allocate region and put into config array.
        Region* region = new Region(regionPtr, regionSizeBytes);
        regionConfigs[regionIdx] = {regionNameCpy, region};
    }

    // Create `StateVector::Config` and wrap it in a `StateVectorParser::Config`
    // shared pointer, which will handle deallocation of all the memory we just
    // allocated.
    StateVector::Config svConfig = {elemConfigs, regionConfigs};
    kConfig.reset(new Config(svConfig, svBacking));

    return SUCCESS;
}

Result StateVectorParser::parseRegion(const std::vector<Token>& kToks,
                                      U32& kIdx,
                                      RegionParse& kRegion,
                                      ConfigInfo* kConfigInfo)
{
    while (kIdx < kToks.size())
    {
        const Token& tok = kToks[kIdx];
        switch (tok.type)
        {
            case TOK_IDENTIFIER:
            {
                // Element token indicates start of element declaration.
                ElementParse elem;
                Result res = StateVectorParser::parseElement(
                    kToks, kIdx, elem, kConfigInfo);
                if (res != SUCCESS)
                {
                    return res;
                }
                kRegion.elems.push_back(elem);
                break;
            }

            case TOK_NEWLINE:
                // Consume newline token.
                ++kIdx;
                break;

            case TOK_SECTION:
                // Section token indicates end of region.
                return SUCCESS;

            default:
                // Unexpected token.
                if (kConfigInfo != nullptr)
                {
                    kConfigInfo->error.lineNum = tok.lineNum;
                    kConfigInfo->error.colNum = tok.colNum;
                    auto iter = gTokenNames.find(tok.type);
                    if (iter == gTokenNames.end())
                    {
                        // Should be unreachable- indicates that `gTokenNames`
                        // is missing a key.
                        return E_KEY;
                    }
                    const std::string& tokTypeName = (*iter).second;
                    kConfigInfo->error.msg =
                        "unexpected " + tokTypeName + " in region `"
                        + kRegion.plainName + "`";
                }
                return E_PARSE;
        }
    }

    return SUCCESS;
}

Result StateVectorParser::parseElement(const std::vector<Token>& kToks,
                                       U32& kIdx,
                                       ElementParse& kElem,
                                       ConfigInfo* kConfigInfo)
{
    // Consume element type token.
    const Token& tokType = kToks[kIdx++];
    kElem.tokType = tokType;

    if (kIdx == kToks.size() || (kToks[kIdx].type == TOK_NEWLINE))
    {
        // End of line or token stream before the element name.
        if (kConfigInfo != nullptr)
        {
            kConfigInfo->error.lineNum = tokType.lineNum;
            kConfigInfo->error.colNum = tokType.colNum;
            kConfigInfo->error.msg = "expected element name after type";
        }
        return E_PARSE;
    }

    // Consume element name token.
    const Token& tokName = kToks[kIdx++];
    if (tokName.type != TOK_IDENTIFIER)
    {
        // Token following element type is not an identifier.
        if (kConfigInfo != nullptr)
        {
            kConfigInfo->error.lineNum = tokName.lineNum;
            kConfigInfo->error.colNum = tokName.colNum;
            kConfigInfo->error.msg = "expected element name";
        }
        return E_PARSE;
    }
    kElem.tokName = tokName;

    return SUCCESS;
}

Result StateVectorParser::allocateElement(const ElementParse& kElem,
                                          StateVector::ElementConfig& kElemInfo,
                                          char*& kBumpPtr)
{
    // Allocate a copy of the element name for the `StateVector::ElementConfig`
    // representing this element.
    char* nameCpy = new char[kElem.tokName.str.size() + 1];
    std::strcpy(nameCpy, kElem.tokName.str.c_str());
    kElemInfo.name = nameCpy;

    if (kElem.tokType.str == "I8")
    {
        I8& backing = *((I8*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<I8>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "I16")
    {
        I16& backing = *((I16*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<I16>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "I32")
    {
        I32& backing = *((I32*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<I32>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "I64")
    {
        I64& backing = *((I64*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<I64>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "U8")
    {
        U8& backing = *((U8*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<U8>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "U16")
    {
        U16& backing = *((U16*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<U16>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "U32")
    {
        U32& backing = *((U32*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<U32>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "U64")
    {
        U64& backing = *((U64*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<U64>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "F32")
    {
        F32& backing = *((F32*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<F32>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "F64")
    {
        F64& backing = *((F64*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<F64>(backing));
        kBumpPtr += sizeof(backing);
    }
    else if (kElem.tokType.str == "bool")
    {
        bool& backing = *((bool*) kBumpPtr);
        kElemInfo.elem = static_cast<IElement*>(new Element<bool>(backing));
        kBumpPtr += sizeof(backing);
    }
    else
    {
        delete[] kElemInfo.name;
        return E_UNREACHABLE;
    }

    return SUCCESS;
}
