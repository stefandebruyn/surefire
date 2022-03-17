#ifndef SF_STATE_MACHINE_PARSER_HPP
#define SF_STATE_MACHINE_PARSER_HPP

#include <istream>
#include <memory>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/ExpressionParser.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/StateMachine.hpp"

namespace StateMachineParser
{
    struct ActionParse
    {
        Token tokRhs;
        std::shared_ptr<ExpressionParser::Parse> lhs;
        Token tokDestState;
    };

    struct BlockParse final
    {
        std::shared_ptr<ExpressionParser::Parse> guard;
        std::shared_ptr<ActionParse> action;
        std::shared_ptr<BlockParse> ifBlock;
        std::shared_ptr<BlockParse> elseBlock;
        std::shared_ptr<BlockParse> next;
    };

    struct StateParse final
    {
        Token tokName;
        std::shared_ptr<BlockParse> entry;
        std::shared_ptr<BlockParse> step;
        std::shared_ptr<BlockParse> exit;
    };

    struct StateVectorElementParse final
    {
        Token tokType;
        Token tokName;
        Token tokAlias;
        std::string alias;
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
        std::vector<StateVectorElementParse> svElems;
        std::vector<LocalElementParse> localElems;
        std::vector<StateParse> states;
        bool hasStateVectorSection;
        bool hasLocalSection;
    };

    Result parse(const std::vector<Token>& kToks,
                 Parse& kParse,
                 ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseLocalSection(TokenIterator& kIt,
                             Parse& kParse,
                             ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseStateVectorSection(TokenIterator& kIt,
                                   Parse& kParse,
                                   ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseStateSection(TokenIterator& kIt,
                             StateParse& kState,
                             ErrorInfo* const kErr);
}

#endif
