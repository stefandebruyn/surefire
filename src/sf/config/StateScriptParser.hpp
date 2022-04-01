#ifndef SF_STATE_SCRIPT_PARSER_HPP
#define SF_STATE_SCRIPT_PARSER_HPP

#include "sf/config/StateMachineParser.hpp"

class StateScriptParse final
{
public:

    struct SectionParse final
    {
        Token tokName;
        Ref<const StateMachineParse::BlockParse> block;
    };

    struct Config final
    {
        Token tokDeltaT;
        Token tokInitState;
        U64 deltaT;
    };

    Vec<StateScriptParse::SectionParse> sections;

    StateScriptParse::Config config;

private:

    friend class StateScriptParser;

    StateScriptParse(const Vec<StateScriptParse::SectionParse>& kSections,
                     const StateScriptParse::Config& kConfig);
};

class StateScriptParser final
{
public:

    StateScriptParser() = delete;

    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateScriptParse>& kParse,
                        ErrorInfo* const kErr);
};

#endif
