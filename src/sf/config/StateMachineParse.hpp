#ifndef SF_STATE_MACHINE_PARSE_HPP
#define SF_STATE_MACHINE_PARSE_HPP

#include <istream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/ExpressionParse.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/StateMachine.hpp"

class StateMachineParse final
{
public:

    struct ActionParse
    {
        Token tokLhs;
        Ref<const ExpressionParse> rhs;
        Token tokDestState;
        Token tokTransitionKeyword;
    };

    struct BlockParse final
    {
        Ref<const ExpressionParse> guard;
        Ref<const StateMachineParse::ActionParse> action;
        Ref<const StateMachineParse::BlockParse> ifBlock;
        Ref<const StateMachineParse::BlockParse> elseBlock;
        Ref<const StateMachineParse::BlockParse> next;
        Ref<const ExpressionParse> assert;
        Token tokElse;
        Token tokAssert;
        Token tokStop;
    };

    struct StateParse final
    {
        Token tokName;
        Ref<const StateMachineParse::BlockParse> entry;
        Ref<const StateMachineParse::BlockParse> step;
        Ref<const StateMachineParse::BlockParse> exit;
    };

    struct StateVectorElementParse final
    {
        Token tokType;
        Token tokName;
        Token tokAlias;
        bool readOnly;
    };

    struct LocalElementParse final
    {
        Token tokType;
        Token tokName;
        Ref<const ExpressionParse> initValExpr;
        bool readOnly;
    };

    Vec<StateMachineParse::StateVectorElementParse> svElems;
    Vec<StateMachineParse::LocalElementParse> localElems;
    Vec<StateMachineParse::StateParse> states;
    bool hasStateVectorSection;
    bool hasLocalSection;

    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateMachineParse>& kParse,
                        ErrorInfo* const kErr);

    static Result parseBlock(TokenIterator kIt,
                             Ref<const StateMachineParse::BlockParse>& kParse,
                             ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    static Result parseLocalSection(
        TokenIterator& kIt,
        Vec<StateMachineParse::LocalElementParse>& kParse,
        ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    static Result parseStateVectorSection(
        TokenIterator& kIt,
        Vec<StateMachineParse::StateVectorElementParse>& kParse,
        ErrorInfo* const kErr);

    /// @note PUBLIC FOR TESTING PURPOSES ONLY.
    static Result parseStateSection(TokenIterator& kIt,
                                    StateMachineParse::StateParse& kParse,
                                    ErrorInfo* const kErr);

private:

    struct MutBlockParse final
    {
        Ref<const ExpressionParse> guard;
        Ref<const StateMachineParse::ActionParse> action;
        Ref<StateMachineParse::MutBlockParse> ifBlock;
        Ref<StateMachineParse::MutBlockParse> elseBlock;
        Ref<StateMachineParse::MutBlockParse> next;
        Ref<const ExpressionParse> assert;
        Token tokElse;
        Token tokAssert;
        Token tokStop;

        void toBlockParse(Ref<const StateMachineParse::BlockParse>& kParse);
    };

    static Result parseAction(
        TokenIterator kIt,
        Ref<const StateMachineParse::ActionParse>& kParse,
        ErrorInfo* const kErr);

    static Result parseBlockRec(TokenIterator kIt,
                                Ref<StateMachineParse::MutBlockParse>& kParse,
                                ErrorInfo* const kErr);


    StateMachineParse(
        const Vec<StateMachineParse::StateVectorElementParse>& kSvElems,
        const Vec<StateMachineParse::LocalElementParse>& kLocalElems,
        const Vec<StateMachineParse::StateParse>& kStates,
        const bool kHasStateVectorSection,
        const bool kHasLocalSection);
};

#endif
