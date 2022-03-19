#ifndef SF_STATE_MACHINE_PARSER_HPP
#define SF_STATE_MACHINE_PARSER_HPP

#include <istream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/ExpressionParser.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/StateMachine.hpp"

namespace StateMachineParser
{
    struct ActionParse
    {
        Token tokRhs;
        Ref<const ExpressionParser::Parse> lhs;
        Token tokDestState;
    };

    struct BlockParse final
    {
        Ref<const ExpressionParser::Parse> guard;
        Ref<StateMachineParser::ActionParse> action;
        Ref<StateMachineParser::BlockParse> ifBlock;
        Ref<StateMachineParser::BlockParse> elseBlock;
        Ref<StateMachineParser::BlockParse> next;
        Ref<const ExpressionParser::Parse> assertion;
    };

    struct StateParse final
    {
        Token tokName;
        Ref<StateMachineParser::BlockParse> entry;
        Ref<StateMachineParser::BlockParse> step;
        Ref<StateMachineParser::BlockParse> exit;
    };

    struct StateVectorElementParse final
    {
        Token tokType;
        Token tokName;
        Token tokAlias;
        String alias;
        bool readOnly;
    };

    struct LocalElementParse final
    {
        Token tokType;
        Token tokName;
        Token tokInitVal;
        bool readOnly;
    };

    struct Parse final
    {
        Vec<StateMachineParser::StateVectorElementParse> svElems;
        Vec<StateMachineParser::LocalElementParse> localElems;
        Vec<StateMachineParser::StateParse> states;
        bool hasStateVectorSection;
        bool hasLocalSection;
    };

    Result parse(const Vec<Token>& kToks,
                 StateMachineParser::Parse& kParse,
                 ErrorInfo* const kErr);

    Result parseBlock(TokenIterator kIt,
                      Ref<StateMachineParser::BlockParse>& kBlock,
                      ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseLocalSection(TokenIterator& kIt,
                             StateMachineParser::Parse& kParse,
                             ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseStateVectorSection(TokenIterator& kIt,
                                   StateMachineParser::Parse& kParse,
                                   ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseStateSection(TokenIterator& kIt,
                             StateMachineParser::StateParse& kState,
                             ErrorInfo* const kErr);
}

#endif
