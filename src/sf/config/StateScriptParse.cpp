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

    // Consume any leading newlines.
    it.eat();

    while (!it.eof())
    {
        if (it.type() != Token::SECTION)
        {
            // Expected a state section.
            ConfigUtil::setError(kErr,
                                 it.tok(),
                                 gErrText,
                                 "expected a section");
            return E_SSP_SEC;
        }

        // Add new section 
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

    // Return final parse.
    kParse.reset(new StateScriptParse(sections));

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

StateScriptParse::StateScriptParse(
    const Vec<StateScriptParse::SectionParse>& kSections) : sections(kSections)
{
}
