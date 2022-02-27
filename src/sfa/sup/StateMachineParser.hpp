#ifndef SFA_STATE_MACHINE_PARSER_HPP
#define SFA_STATE_MACHINE_PARSER_HPP

#include <istream>
#include <memory>

#include "sfa/core/StateMachine.hpp"
#include "sfa/core/StateVector.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"
#include "sfa/sup/ConfigErrorInfo.hpp"
#include "sfa/sup/TokenIterator.hpp"

namespace StateMachineParser
{
    struct ExpressionParse final
    {
        Token tokData;
        ExpressionParse* left;
        ExpressionParse* right;
        bool func;
    };

    struct ActionParse
    {
        Token tokRhs;
        ExpressionParse tokLhs;
        Token destState;
    };

    struct BlockParse final
    {
        ExpressionParse* guard;
        ActionParse* action;
        BlockParse* ifBlock;
        BlockParse* elseBlock;
        BlockParse* next;
    };

    struct StateParse final
    {
        Token tokName;
        BlockParse* entry;
        BlockParse* step;
        BlockParse* exit;
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
        bool hasLocalSection;
        bool hasStateVectorSection;
    };

    class Config final
    {
        // todo
    };

    Result parse(std::istream& kIs,
                 StateVector& kSv,
                 std::shared_ptr<Config>& kConfig,
                 ConfigErrorInfo* kConfigErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseLocalSection(TokenIterator& kIt,
                             Parse& kParse,
                             ConfigErrorInfo* kConfigErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    Result parseStateVectorSection(TokenIterator& kIt,
                                   const StateVector& kSv,
                                   Parse& kParse,
                                   ConfigErrorInfo* kConfigErr);
}

#endif
