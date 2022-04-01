#include "sf/config/StateVectorParser.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state vector config error";

/////////////////////////////////// Public /////////////////////////////////////

const Vec<String> StateVectorParser::ALL_REGIONS;

Result StateVectorParser::parse(const Vec<Token>& kToks,
                                Ref<const StateVectorParse>& kParse,
                                ErrorInfo* const kErr,
                                const Vec<String> kRegions)
{
    // Create iterator for token vector.
    TokenIterator it(kToks.begin(), kToks.end());

    // Vector of parsed regions.
    Vec<StateVectorParse::RegionParse> regions;

    while (!it.eof())
    {
        switch (it.type())
        {
            case Token::NEWLINE:
                // Eat newlines.
                it.take();
                break;

            case Token::SECTION:
            {
                // Region section.

                // Extract plain name of region (without the section brakets).
                StateVectorParse::RegionParse region{};
                region.plainName = it.str().substr(1, (it.str().size() - 2));

                // Only parse region if all regions were selected for parsing or
                // the region name appears in the passed vector.
                if ((kRegions == ALL_REGIONS)
                    || (std::find(kRegions.begin(),
                                  kRegions.end(),
                                  region.plainName) != kRegions.end()))
                {
                    const Result res =
                        StateVectorParser::parseRegion(it, region, kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }

                    // Add region to parse.
                    regions.push_back(region);
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
                ErrorInfo::set(kErr, it.tok(), gErrText, "unexpected token");
                return E_SVP_TOK;
        }
    }

    // If specific regions were selected for parsing, check that all specified
    // regions existed in the config.
    for (const String& regionName : kRegions)
    {
        bool found = false;
        for (const StateVectorParse::RegionParse& regionParse : regions)
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
                kErr->text = gErrText;
                kErr->subtext = ("region `" + regionName
                                 + "` does not exist in config");
            }

            return E_SVP_RGN;
        }
    }

    // Return final parse.
    kParse.reset(new StateVectorParse(regions));

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

StateVectorParse::StateVectorParse(Vec<StateVectorParse::RegionParse>& kRegions)
    : regions(kRegions)
{
}

Result StateVectorParser::parseRegion(TokenIterator& kIt,
                                      StateVectorParse::RegionParse& kRegion,
                                      ErrorInfo* const kErr)
{
    // Assert that token iterator is currently positioned at a section.
    SF_SAFE_ASSERT(kIt.type() == Token::SECTION);

    // Take section name.
    kRegion.tokName = kIt.take();

    // Parse elements until EOF or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        // Add new element to region.
        kRegion.elems.push_back(StateVectorParse::ElementParse{});
        StateVectorParse::ElementParse& elem = kRegion.elems.back();

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ErrorInfo::set(kErr, kIt.tok(), gErrText, "expected element type");
            return E_SVP_ELEM_TYPE;
        }

        // Take element type.
        elem.tokType = kIt.take();

        // Check that tokens remain.
        if (kIt.eof())
        {
            ErrorInfo::set(kErr, elem.tokType, gErrText,
                           "expected element name after type");
            return E_SVP_ELEM_NAME;
        }

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ErrorInfo::set(kErr, kIt.tok(), gErrText,
                           "expected element type");
            return E_SVP_ELEM_NAME;
        }

        // Take element name.
        elem.tokName = kIt.take();
    }

    return SUCCESS;
}
