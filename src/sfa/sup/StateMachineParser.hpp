#ifndef SFA_STATE_MACHINE_PARSER_HPP
#define SFA_STATE_MACHINE_PARSER_HPP

#include <istream>
#include <memory>

#include "sfa/core/StateMachine.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"
#include "sfa/sup/ConfigErrorInfo.hpp"
#include "sfa/sup/TokenIterator.hpp"

namespace StateMachineParser
{
    struct ExpressionParse final
    {
        Token tokData;
        std::shared_ptr<ExpressionParse> left;
        std::shared_ptr<ExpressionParse> right;
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
        std::shared_ptr<ExpressionParse> guard;
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
        StateVectorElementParse();

        Token tokType;
        Token tokName;
        Token tokAlias;
        std::string alias;
        bool readOnly;
    };

    struct LocalElementParse final
    {
        LocalElementParse();

        Token tokType;
        Token tokName;
        Token tokInitVal;
        bool readOnly;
    };

    struct Parse final
    {
        Parse();

        std::vector<StateVectorElementParse> svElems;
        std::vector<LocalElementParse> localElems;
        std::vector<StateParse> states;
        bool hasLocalSection;
    };

    class Config final
    {
        // todo
    };

    Result parse(std::istream& kIs,
                 std::shared_ptr<Config>& kConfig,
                 ConfigErrorInfo* kConfigErr);

    /// @note This function is public for testing purposes only. It should not
    /// be called by the user.
    Result parseLocalSection(TokenIterator& kIt,
                             Parse& kParse,
                             ConfigErrorInfo* kConfigErr);
}

#endif
