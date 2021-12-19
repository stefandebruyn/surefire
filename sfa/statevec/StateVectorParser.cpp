#include <fstream>
#include <iostream> // rm later

#include "sfa/statevec/StateVectorParser.hpp"

const std::regex StateVectorParser::mRegionSectionRegex(
    "REGION/([a-zA-Z][a-zA-Z0-9_]*)");

Result StateVectorParser::parse(const std::string kFilePath,
                                StateVector::Config& kConfig,
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
                                StateVector::Config& kConfig,
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
                                    StateVector::Config& kConfig,
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
                    region.name = match[1].str();
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
                        // is incomplete.
                        return E_KEY;
                    }
                    const std::string& tokTypeName = (*iter).second;
                    kConfigInfo->error.msg = "unexpected " + tokTypeName;
                }
                return E_PARSE;
        }
    }

    for (RegionParse& region : stateVec.regions)
    {
        std::cout << region.name << std::endl;
        for (ElementParse& elem : region.elems)
        {
            std::cout << "  " << elem.type << " " << elem.name << std::endl;
        }
    }

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
                        // is incomplete.
                        return E_KEY;
                    }
                    const std::string& tokTypeName = (*iter).second;
                    kConfigInfo->error.msg =
                        "unexpected " + tokTypeName + " in region `"
                        + kRegion.name + "`";
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
    kElem.type = tokType.str;

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
    kElem.name = tokName.str;

    return SUCCESS;
}
