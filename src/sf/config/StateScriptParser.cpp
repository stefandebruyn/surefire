#include "sf/core/Assert.hpp"
#include "sf/config/ConfigUtil.hpp"
#include "sf/config/StateScriptParser.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "state script parser error";

} // Anonymous namespace

/////////////////////////////////// Public /////////////////////////////////////

Result StateScriptParser::parse(const Vec<Token>& kToks,
                                StateScriptParser::Parse& kParse,
                                ErrorInfo* const kErr)
{
    TokenIterator it(kToks.begin(), kToks.end());
    StateScriptParser::Parse parse = {};

    // Consume any leading newlines.
    it.eat();

    while (!it.eof())
    {
        if (it.type() != Token::SECTION)
        {
            // Expected a state section.
            ConfigUtil::setError(kErr, it.tok(), errText, "expected a section");
            return E_SSP_SEC;
        }

        // Add new section parse.
        parse.sections.push_back(StateScriptParser::SectionParse{});
        StateScriptParser::SectionParse& section = parse.sections.back();

        // Take section token.
        section.tokName = it.take();

        // Find end of section.
        const U32 idxEnd = it.next({Token::SECTION});

        // Parse section contents as a single block.
        const Result res = StateMachineParser::parseBlock(
            it.slice(it.idx(), idxEnd),
            section.block,
            kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Jump to end of section.
        it.seek(idxEnd);
    }

    // If we got this far, the parse is valid- return it.
    kParse = parse;

    return SUCCESS;
}
