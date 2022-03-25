#include <climits>
#include <cstdlib>

#include "sf/core/Assert.hpp"
#include "sf/config/ConfigUtil.hpp"
#include "sf/config/StateScriptParse.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state script parser error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateScriptParse::parse(const Vec<Token>& kToks,
                               Ref<const StateScriptParse>& kParse,
                               ErrorInfo* const kErr)
{
    // Create iterator for token vector.
    TokenIterator it(kToks.begin(), kToks.end());

    // Vector of parsed sections.
    Vec<StateScriptParse::SectionParse> sections;

    // Parsed config options.
    StateScriptParse::Config config{};

    // Consume any leading newlines.
    it.eat();

    while (!it.eof())
    {
        if (it.type() != Token::SECTION)
        {
            // Expected a section.
            ConfigUtil::setError(kErr,
                                 it.tok(),
                                 gErrText,
                                 "expected a section");
            return E_SSP_SEC;
        }

        // Config section gets special treatment.
        if (it.str() == LangConst::sectionConfig)
        {
            // Take section token.
            it.take();

            // Process config options, which are annotations.
            while (it.type() == Token::ANNOTATION)
            {
                std::smatch match;
                if (std::regex_match(it.str(),
                                     match,
                                     LangConst::annotationDeltaTRegex))
                {
                    // Delta T option.
                    const String deltaTStr = match[1].str();
                    config.deltaT = std::strtoll(deltaTStr.c_str(),
                                                 nullptr,
                                                 10);

                    // Check that delta T is nonzero.
                    if (config.deltaT == 0)
                    {
                        ConfigUtil::setError(kErr,
                                             it.tok(),
                                             gErrText,
                                             "delta T must be nonzero");
                        return E_SSP_DT;
                    }

                    // Check that delta T is not too large.
                    if (config.deltaT >= LLONG_MAX)
                    {
                        ConfigUtil::setError(kErr,
                                             it.tok(),
                                             gErrText,
                                             "delta T is too large");
                        return E_SSP_DT;
                    }
                }
                else
                {
                    // Unknown config option.
                    ConfigUtil::setError(
                        kErr,
                        it.tok(),
                        gErrText,
                        "unknown config option `" + it.str() + "`");
                    return E_SSP_CONFIG;
                }

                // Take option annotation.
                it.take();
            }
        }
        else
        {
            // Parse state section.
            sections.push_back(StateScriptParse::SectionParse{});
            StateScriptParse::SectionParse& section = sections.back();

            // Take section token.
            section.tokName = it.take();

            // Find end of section.
            const U32 idxEnd = it.next({Token::SECTION});

            // Parse section contents as a single block.
            const Result res = StateMachineParse::parseBlock(it.slice(it.idx(),
                                                                      idxEnd),
                                                             section.block,
                                                             kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Jump to end of section.
            it.seek(idxEnd);
        }
    }

    // Check that a delta T was specified.
    if (config.deltaT == 0)
    {
        if (kErr != nullptr)
        {
            kErr->text = gErrText;
            kErr->subtext =
                "`@DELTA_T` was not specified in `[CONFIG]` section";
        }
        return E_SSP_DT;
    }

    // Return final parse.
    kParse.reset(new StateScriptParse(sections, config));

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

StateScriptParse::StateScriptParse(
    const Vec<StateScriptParse::SectionParse>& kSections,
    const StateScriptParse::Config& kConfig) :
    sections(kSections), config(kConfig)
{
}
