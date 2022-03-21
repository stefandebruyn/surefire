#ifndef SF_STATE_SCRIPT_PARSE_HPP
#define SF_STATE_SCRIPT_PARSE_HPP

#include "sf/config/StateMachineParse.hpp"

class StateScriptParse final
{
public:

    struct SectionParse final
    {
        Token tokName;
        Ref<const StateMachineParse::BlockParse> block;
    };

    Vec<StateScriptParse::SectionParse> sections;

    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateScriptParse>& kParse,
                        ErrorInfo* const kErr);

private:

    StateScriptParse(const Vec<StateScriptParse::SectionParse>& kSections);
};

#endif
