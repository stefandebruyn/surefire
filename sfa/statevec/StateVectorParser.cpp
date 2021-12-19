#include <fstream>
#include <iostream> // rm later

#include "sfa/statevec/StateVectorParser.hpp"

const std::regex StateVectorParser::mRegionSectionRegex(
    "REGION/([a-zA-Z][a-zA-Z0-9_]*)");

Result StateVectorParser::parse(const std::string kFilePath,
                                StateVector::Config& kConfig,
                                ConfigErrorInfo* kErrInfo)
{
    std::ifstream ifs(kFilePath);
    if (ifs.is_open() == false)
    {
        return E_OPEN_FILE;
    }

    return StateVectorParser::parse(ifs, kConfig, kErrInfo);
}

Result StateVectorParser::parse(std::istream& kIs,
                                StateVector::Config& kConfig,
                                ConfigErrorInfo* kErrInfo)
{
    std::vector<Token> toks;
    Result res = Tokenizer::tokenize(kIs, toks, kErrInfo);
    if (res != SUCCESS)
    {
        return res;
    }

    return StateVectorParser::parseImpl(toks, kConfig, kErrInfo);
}

Result StateVectorParser::parseImpl(const std::vector<Token>& kToks,
                                    StateVector::Config& kConfig,
                                    ConfigErrorInfo* kErrInfo)
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
                        kToks, idx, region, kErrInfo);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                    stateVec.regions.push_back(region);
                }
                else
                {
                    // Unknown section.
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
                                      ConfigErrorInfo* kErrInfo)
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
                    kToks, kIdx, elem, kErrInfo);
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
                return E_PARSE;
        }
    }

    return SUCCESS;
}

Result StateVectorParser::parseElement(const std::vector<Token>& kToks,
                                       U32& kIdx,
                                       ElementParse& kElem,
                                       ConfigErrorInfo* kErrInfo)
{
    // Consume element type token.
    kElem.type = kToks[kIdx++].str;

    if (kIdx == kToks.size())
    {
        // Unexpected end of token stream.
        return E_PARSE;
    }

    // Consume element name token.
    kElem.name = kToks[kIdx++].str;

    return SUCCESS;
}
