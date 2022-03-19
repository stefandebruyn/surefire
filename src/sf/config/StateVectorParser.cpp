#include "sf/config/ConfigUtil.hpp"
#include "sf/config/StateVectorParser.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "state vector config error";

Result parseRegion(TokenIterator& kIt,
                   StateVectorParser::RegionParse& kRgn,
                   ErrorInfo* const kErr)
{
    // Assert that token iterator is currently positioned at a section.
    SF_ASSERT(kIt.type() == Token::SECTION);

    // Take section name.
    kRgn.tokName = kIt.take();

    // Parse elements until EOF or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        // Add new element to region.
        kRgn.elems.push_back(StateVectorParser::ElementParse{});
        StateVectorParser::ElementParse& elem = kRgn.elems.back();

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SVP_ELEM_TYPE;
        }

        // Take element type.
        elem.tokType = kIt.take();

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(kErr, elem.tokType, errText,
                                 "expected element name after type");
            return E_SVP_ELEM_NAME;
        }

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SVP_ELEM_NAME;
        }

        // Take element name.
        elem.tokName = kIt.take();
    }

    return SUCCESS;
}

} // Anonymous namespace

/////////////////////////////////// Public /////////////////////////////////////

const Vec<String> StateVectorParser::ALL_REGIONS;

Result StateVectorParser::parse(const Vec<Token>& kToks,
                                StateVectorParser::Parse& kParse,
                                ErrorInfo* const kErr,
                                const Vec<String> kRgns)
{
    StateVectorParser::Parse parse = {};
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
                StateVectorParser::RegionParse region = {};
                region.plainName = it.str().substr(1, (it.str().size() - 2));
                if ((kRgns == ALL_REGIONS)
                    || (std::find(kRgns.begin(), kRgns.end(), region.plainName)
                        != kRgns.end()))
                {
                    const Result res = parseRegion(it, region, kErr);
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
                SF_ASSERT(false);
        }
    }

    // If specific regions were selected for parsing, check that all specified
    // regions existed in the config.
    for (const String& regionName : kRgns)
    {
        bool found = false;
        for (const StateVectorParser::RegionParse& regionParse : parse.regions)
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
            if (kErr != nullptr)
            {
                kErr->text = errText;
                kErr->subtext =
                    "region `" + regionName + "` does not exist in config";
            }
            return E_SVP_RGN;
        }
    }

    kParse = parse;
    return SUCCESS;
}
