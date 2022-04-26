#include "sf/core/Assert.hpp"
#include "sf/config/StateScriptParser.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state script parser error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateScriptParser::parse(const Vec<Token>& kToks,
                                Ref<const StateScriptParse>& kParse,
                                ErrorInfo* const kErr)
{
    // Create iterator for token vector.
    TokenIterator it(kToks.begin(), kToks.end());

    // Vector of parsed sections.
    Vec<StateScriptParse::SectionParse> sections;

    // Parsed config options.
    StateScriptParse::Config config;
    config.deltaT = 0;

    // Consume any leading newlines.
    it.eat();

    while (!it.eof())
    {
        if (it.type() != Token::SECTION)
        {
            // Expected a section.
            ErrorInfo::set(kErr, it.tok(), gErrText, "expected a section");
            return E_SSP_SEC;
        }

        // Options section gets special treatment.
        if (it.str() == LangConst::sectionOptions)
        {
            // Take section token.
            it.take();

            // Process config options, which are identifiers possibly followed
            // by other tokens.
            while (it.type() == Token::IDENTIFIER)
            {
                if (it.str() == LangConst::optDeltaT)
                {
                    // Delta T option.

                    // Take identifier token.
                    const Token& tokId = it.take();

                    // Check that next token, which should be the delta T value,
                    // is a constant.
                    if (it.type() != Token::CONSTANT)
                    {
                        ErrorInfo::set(kErr, tokId, gErrText,
                                       ("expected value after `" + tokId.str
                                        + "`"));
                        return E_SSP_DT;
                    }

                    // Take delta T value.
                    config.tokDeltaT = it.take();
                }
                else if (it.str() == LangConst::optInitState)
                {
                    // Initial state option.

                    // Take identifier token.
                    const Token& tokId = it.take();

                    // Check that next token, which should be the initial state
                    // name, is an identifier.
                    if (it.type() != Token::IDENTIFIER)
                    {
                        ErrorInfo::set(kErr, tokId, gErrText,
                                       ("expected state name after `"
                                        + tokId.str + "`"));
                        return E_SSP_STATE;
                    }

                    // Take initial state name.
                    config.tokInitState = it.take();
                }
                else
                {
                    // Unknown config option.
                    ErrorInfo::set(kErr, it.tok(), gErrText,
                                   ("unknown config option `" + it.str()
                                    + "`"));
                    return E_SSP_CONFIG;
                }
            }
        }
        else
        {
            // Parse state section.
            sections.push_back(StateScriptParse::SectionParse());
            StateScriptParse::SectionParse& section = sections.back();

            // Take section token.
            section.tokName = it.take();

            // Find end of section.
            const U32 idxEnd = it.next({Token::SECTION});

            // Parse section contents as a single block.
            const Result res = StateMachineParser::parseBlock(
                it.slice(it.idx(), idxEnd), section.block, kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Jump to end of section.
            it.seek(idxEnd);
        }
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
