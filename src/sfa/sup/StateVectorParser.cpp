#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"
#include "sfa/sup/StateVectorParser.hpp"
#include "sfa/sup/TokenIterator.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace StateVectorParser
{
    const char* const errText = "state vector config error";

    Result parseRegion(TokenIterator& kIt,
                       RegionParse& kRgn,
                       ConfigErrorInfo* kConfigErr);
}

Result StateVectorParser::parseRegion(TokenIterator& kIt,
                                      RegionParse& kRgn,
                                      ConfigErrorInfo* kConfigErr)
{
    // Assert that token iterator is currently positioned at a section.
    SFA_ASSERT(kIt.type() == Token::SECTION);

    // Take section name.
    kRgn.tokName = kIt.take();

    // Parse elements until EOF or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        // Add new element to region.
        kRgn.elems.push_back(ElementParse{});
        ElementParse& elem = kRgn.elems.back();

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SVP_ELEM_TYPE;
        }

        // Take element type.
        elem.tokType = kIt.take();

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(kConfigErr, elem.tokType, errText,
                                 "expected element name after type");
            return E_SVP_ELEM_NAME;
        }

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SVP_ELEM_NAME;
        }

        // Take element name.
        elem.tokName = kIt.take();
    }

    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

const std::vector<std::string> StateVectorParser::ALL_REGIONS;

Result StateVectorParser::parse(const std::vector<Token>& kToks,
                                Parse& kParse,
                                ConfigErrorInfo* kConfigErr,
                                const std::vector<std::string> kRgns)
{
    Parse parse = {};
    TokenIterator it(kToks.begin(), kToks.end());

    while (!it.eof())
    {
        switch (it.type())
        {
            case Token::NEWLINE:
                it.take();
                break;

            case Token::SECTION:
            {
                // Region section.

                // Only parse region if `ALL_REGIONS` was passed or the region
                // name appears in the passed vector.
                RegionParse region = {};
                region.plainName = it.str().substr(1, (it.str().size() - 2));
                if ((kRgns == ALL_REGIONS)
                    || (std::find(kRgns.begin(), kRgns.end(), region.plainName)
                        != kRgns.end()))
                {
                    const Result res = parseRegion(it, region, kConfigErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                    parse.regions.push_back(region);
                }
                else
                {
                    // Region was not selected for parsing- skip to EOF or next
                    // section.
                    it.take();
                    const U32 idxNext = it.next({Token::SECTION});
                    it.seek(idxNext);
                }
                break;
            }

            default:
                // Unexpected token.
                SFA_ASSERT(false);
        }
    }

    // If specific regions were selected for parsing, check that all specified
    // regions existed in the config.
    for (const std::string& regionName : kRgns)
    {
        bool found = false;
        for (const RegionParse& regionParse : parse.regions)
        {
            if (regionParse.plainName == regionName)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Specified region does not exist in config.
            if (kConfigErr != nullptr)
            {
                kConfigErr->text = errText;
                kConfigErr->subtext =
                    "region `" + regionName + "` does not exist in config";
            }
            return E_SVP_RGN;
        }
    }

    kParse = parse;
    return SUCCESS;
}
