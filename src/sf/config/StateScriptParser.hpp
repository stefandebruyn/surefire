#ifndef SF_STATE_SCRIPT_PARSER_HPP
#define SF_STATE_SCRIPT_PARSER_HPP

#include "sf/config/StateMachineParser.hpp"

namespace StateScriptParser
{
    struct SectionParse final
    {
        Token tokName;
        Ref<StateMachineParser::BlockParse> block;
    };

    struct Parse final
    {
        Vec<StateScriptParser::SectionParse> sections;
    };

    Result parse(const Vec<Token>& kToks,
                 StateScriptParser::Parse& kParse,
                 ErrorInfo* const kErr);
}

#endif
