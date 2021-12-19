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

    // Delete `ElementInfo` array.
    delete[] mSvConfig.elems;

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
    StateVectorParse stateVec;

    // Process token stream and populate `stateVec`.
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
                    stateVec.regions.push_back(region);
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

    // Count the number of elements in the state vector and its size in bytes.
    U32 elemCnt = 0;
    U32 svSizeBytes = 0;
    for (const RegionParse& region : stateVec.regions)
    {
        for (const ElementParse& elem : region.elems)
        {
            ++elemCnt;
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

    // Allocate elements array for `StateVector::Config`.
    StateVector::ElementInfo* elemInfos =
        new StateVector::ElementInfo[elemCnt + 1];
    elemInfos[elemCnt] = {nullptr, nullptr}; // Null terminator

    // Allocate backing storage for state vector.
    char* svBacking = new char[svSizeBytes];
    std::memset(svBacking, 0, svSizeBytes);

    // Allocate element objects.
    char* bumpPtr = svBacking;
    U32 elemIdx = 0;
    for (const RegionParse& region : stateVec.regions)
    {
        for (const ElementParse& elem : region.elems)
        {
            Result res = StateVectorParser::allocateElement(
                elem, elemInfos[elemIdx++], bumpPtr);
            if (res != SUCCESS)
            {
                delete[] elemInfos;
                delete[] svBacking;
                return res;
            }
        }
    }

    // Create `StateVector::Config` and wrap it in a `StateVectorParser::Config`
    // shared pointer to handle deallocation.
    StateVector::Config svConfig = {elemInfos};
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
                                          StateVector::ElementInfo& kElemInfo,
                                          char*& kBumpPtr)
{
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
