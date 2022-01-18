#include <fstream>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "sfa/sv/StateVectorParser.hpp"

const std::vector<std::string> StateVectorParser::ALL_REGIONS;

StateVectorParser::Config::Config(const StateVector::Config kSvConfig,
                                  const char* const kSvBacking) :
    mSvConfig(kSvConfig), mSvBacking(kSvBacking)
{
}

StateVectorParser::Config::~Config()
{
    // Delete name string and object for each element.
    for (U32 i = 0; mSvConfig.elems[i].name != nullptr; ++i)
    {
        delete[] mSvConfig.elems[i].name;
        delete mSvConfig.elems[i].elem;
    }

    // Delete name string and object for each region.
    for (U32 i = 0; mSvConfig.regions[i].name != nullptr; ++i)
    {
        delete[] mSvConfig.regions[i].name;
        delete mSvConfig.regions[i].region;
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
                                ConfigErrorInfo* kConfigErr,
                                const std::vector<std::string> kRegions)
{
    std::ifstream ifs(kFilePath);
    if (ifs.is_open() == false)
    {
        if (kConfigErr != nullptr)
        {
            kConfigErr->msg = "failed to open file: " + kFilePath;
        }
        return E_FILE;
    }

    if (kConfigErr != nullptr)
    {
        kConfigErr->filePath = kFilePath;
    }

    return StateVectorParser::parse(ifs, kConfig, kConfigErr, kRegions);
}

Result StateVectorParser::parse(std::istream& kIs,
                                std::shared_ptr<Config>& kConfig,
                                ConfigErrorInfo* kConfigErr,
                                const std::vector<std::string> kRegions)
{
    std::vector<Token> toks;
    Result res = ConfigTokenizer::tokenize(kIs, toks, kConfigErr);
    if (res != SUCCESS)
    {
        return res;
    }

    return StateVectorParser::parseImpl(toks, kConfig, kConfigErr, kRegions);
}

Result StateVectorParser::parseImpl(const std::vector<Token>& kToks,
                                    std::shared_ptr<Config>& kConfig,
                                    ConfigErrorInfo* kConfigErr,
                                    const std::vector<std::string>& kRegions)
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
                    // If the caller provided a list of regions to parse and
                    // this isn't one of them, skip to the next region section
                    // or end of token stream.
                    const std::string regionPlainName = match[1].str();
                    if ((kRegions.size() > 0)
                        && (std::find(kRegions.begin(),
                                      kRegions.end(),
                                      regionPlainName) == kRegions.end()))
                    {
                        while ((idx < kToks.size())
                               && (kToks[idx].type != TOK_SECTION))
                        {
                            ++idx;
                        }
                        continue;
                    }

                    // Parse region section- check that region name is unique.
                    for (const RegionParse& regionParse : parse.regions)
                    {
                        if (regionParse.tokName.str == tok.str)
                        {
                            // Name is not unique.
                            if (kConfigErr != nullptr)
                            {
                                kConfigErr->lineNum = tok.lineNum;
                                kConfigErr->colNum = tok.colNum;
                                std::stringstream ss;
                                ss << "reuse of region name `"
                                   << regionParse.plainName
                                   << "` (previously used on line "
                                   << regionParse.tokName.lineNum << ")";
                                kConfigErr->msg = ss.str();
                            }
                            return E_PARSE;
                        }
                    }

                    // Add new `RegionParse` to regions vector prior to
                    // `parseRegion` call so that `parseElement` can see
                    // previously parsed elements in the same region for the
                    // purpose of name uniqueness checking.
                    parse.regions.push_back(RegionParse());
                    RegionParse& regionParse = parse.regions.back();
                    regionParse.tokName = tok;
                    regionParse.plainName = regionPlainName;

                    const Result res = StateVectorParser::parseRegion(
                        kToks, idx, parse, regionParse, kConfigErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                }
                else
                {
                    // Unknown section.
                    if (kConfigErr != nullptr)
                    {
                        kConfigErr->lineNum = tok.lineNum;
                        kConfigErr->colNum = tok.colNum;
                        kConfigErr->msg = "invalid section name";
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
                if (kConfigErr != nullptr)
                {
                    kConfigErr->lineNum = tok.lineNum;
                    kConfigErr->colNum = tok.colNum;
                    kConfigErr->msg = "expected a section";
                }
                return E_PARSE;
        }
    }

    // At this point we have a potentially valid state vector parsing- now we
    // try compiling it into a `StateVector::Config`.

    // Count the number of elements, regions, and bytes in the state vector.
    U32 elemCnt = 0;
    const U32 regionCnt = parse.regions.size();
    U32 svSizeBytes = 0;
    for (const RegionParse& region : parse.regions)
    {
        elemCnt += region.elems.size();
        for (const ElementParse& elem : region.elems)
        {
            svSizeBytes += elem.sizeBytes;
        }
    }

    // Allocate array for element configs.
    StateVector::ElementConfig* elemConfigs =
        new StateVector::ElementConfig[elemCnt + 1];
    elemConfigs[elemCnt] = {nullptr, nullptr}; // Null terminator.

    // Allocate array for region configs.
    StateVector::RegionConfig* regionConfigs =
        new StateVector::RegionConfig[regionCnt + 1];
    regionConfigs[regionCnt] = {nullptr, nullptr}; // Null terminator.

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
                                      const Parse& kParse,
                                      RegionParse& kRegion,
                                      ConfigErrorInfo* kConfigErr)
{
    while (kIdx < kToks.size())
    {
        const Token& tok = kToks[kIdx];
        switch (tok.type)
        {
            case TOK_IDENTIFIER:
            {
                // Element token indicates start of element declaration.
                kRegion.elems.push_back(ElementParse());
                ElementParse& elemParse = kRegion.elems.back();
                Result res = StateVectorParser::parseElement(
                    kToks, kIdx, kParse, elemParse, kConfigErr);
                if (res != SUCCESS)
                {
                    return res;
                }
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
                if (kConfigErr != nullptr)
                {
                    // Get human-readable name of unexpected token type.
                    auto iter = gTokenNames.find(tok.type);
                    if (iter == gTokenNames.end())
                    {
                        // Should be unreachable- indicates that `gTokenNames`
                        // is missing a key.
                        return E_KEY;
                    }
                    const std::string& tokTypeName = (*iter).second;

                    // Populate config error info.
                    kConfigErr->lineNum = tok.lineNum;
                    kConfigErr->colNum = tok.colNum;
                    kConfigErr->msg =
                        "expected element or region, got " + tokTypeName;
                }
                return E_PARSE;
        }
    }

    return SUCCESS;
}

Result StateVectorParser::parseElement(const std::vector<Token>& kToks,
                                       U32& kIdx,
                                       const Parse& kParse,
                                       ElementParse& kElem,
                                       ConfigErrorInfo* kConfigErr)
{
    // Consume element type token.
    const Token& tokType = kToks[kIdx++];
    kElem.tokType = tokType;

    // Check that type is valid.
    auto iter = mElemTypeSize.find(tokType.str);
    if (iter == mElemTypeSize.end())
    {
        // Unknown element type.
        if (kConfigErr != nullptr)
        {
            kConfigErr->lineNum = tokType.lineNum;
            kConfigErr->colNum = tokType.colNum;
            kConfigErr->msg =
                "unknown type `" + tokType.str + "`";
        }
        return E_PARSE;
    }

    // Set element size.
    kElem.sizeBytes = (*iter).second;

    // Consume any newline tokens following the element type.
    while ((kIdx < kToks.size()) && (kToks[kIdx].type == TOK_NEWLINE))
    {
        ++kIdx;
    }

    if (kIdx == kToks.size() || (kToks[kIdx].type != TOK_IDENTIFIER))
    {
        // Expected element name but got end of token stream or non-identifier
        // token.
        if (kConfigErr != nullptr)
        {
            if (kIdx == kToks.size())
            {
                // End of token stream error message will point to the type
                // token.
                kConfigErr->lineNum = tokType.lineNum;
                kConfigErr->colNum = tokType.colNum;
                kConfigErr->msg =
                    "expected element name after type `" + tokType.str + "`";
            }
            else
            {
                // Non-identifier token error message will point to the
                // unexpected token.

                // Get human-readable name of unexpected token type.
                const Token& tokUnexpect = kToks[kIdx];
                auto iter = gTokenNames.find(tokUnexpect.type);
                if (iter == gTokenNames.end())
                {
                    // Should be unreachable- indicates that `gTokenNames`
                    // is missing a key.
                    return E_KEY;
                }
                const std::string& tokUnexpectName = (*iter).second;

                // Populate config error info.
                kConfigErr->lineNum = tokUnexpect.lineNum;
                kConfigErr->colNum = tokUnexpect.colNum;
                kConfigErr->msg =
                    "expected element name after type `" + tokType.str
                    + "`, got " + tokUnexpectName;
            }
        }

        return E_PARSE;
    }

    // Consume element name token.
    const Token& tokName = kToks[kIdx++];

    // Check that element name is unique.
    for (const RegionParse& regionParse : kParse.regions)
    {
        for (const ElementParse& elemParse : regionParse.elems)
        {
            if (elemParse.tokName.str == tokName.str)
            {
                // Name is not unique.
                if (kConfigErr != nullptr)
                {
                    kConfigErr->lineNum = tokName.lineNum;
                    kConfigErr->colNum = tokName.colNum;
                    std::stringstream ss;
                    ss << "reuse of element name `" << tokName.str + "`"
                       << " (previously used on line "
                       << elemParse.tokName.lineNum << ")";
                    kConfigErr->msg = ss.str();
                }
                return E_PARSE;
            }
        }
    }

    // Assign name token to element parse. Do this after checking for duplicate
    // element names to prevent comparing names to themselves.
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
