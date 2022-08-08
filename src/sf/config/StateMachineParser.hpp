////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/StateMachineParser.hpp
/// @brief State machine DSL parser.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_MACHINE_PARSER_HPP
#define SF_STATE_MACHINE_PARSER_HPP

#include <istream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/ExpressionParser.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/StateMachine.hpp"

///
/// @brief Parse of a state machine.
///
class StateMachineParse final
{
public:

    ///
    /// @brief Parse of an action. The type of action depends on member values:
    ///
    ///   * Assignment action - tokLhs is non-empty (i.e., the string is nonzero
    ///                         in length), rhs is non-null
    ///   * Transition action - tokDestState and tokTransitionKeyword are
    ///                         non-empty
    ///
    struct ActionParse
    {
        ///
        /// @brief If assignment action, LHS of assignment operator.
        ///
        Token tokLhs;

        ///
        /// @brief If assignment action, RHS of assignment operator.
        ///
        Ref<const ExpressionParse> rhs;

        ///
        /// @brief If transition action, destination state identifier.
        ///
        Token tokDestState;

        ///
        /// @brief If transition action, transition keyword.
        ///
        /// @remark This is used to generate error messages and has no effect
        /// on how the action is compiled.
        ///
        Token tokTransitionKeyword;
    };

    ///
    /// @brief Parse of a block. Like StateMachine::Block, the parse is
    /// represented as a tree.
    ///
    struct BlockParse final
    {
        ///
        /// @brief Block guard conditional, or null if none.
        ///
        Ref<const ExpressionParse> guard;

        ///
        /// @brief Block action, or null if none.
        ///
        Ref<const StateMachineParse::ActionParse> action;

        ///
        /// @brief Block under if branch, or null if none.
        ///
        Ref<const StateMachineParse::BlockParse> ifBlock;

        ///
        /// @brief Block under else branch, or null if none.
        ///
        Ref<const StateMachineParse::BlockParse> elseBlock;

        ///
        /// @brief Next block in the control flow, or null if none.
        ///
        Ref<const StateMachineParse::BlockParse> next;

        ///
        /// @brief Assertion statement, or null if none.
        ///
        /// @remark This is only used in state scripts.
        ///
        Ref<const ExpressionParse> assert;

        ///
        /// @brief Else keyword if the block has an else branch.
        ///
        /// @remark This is used to generate error messages and has no effect
        /// on how the block is compiled.
        ///
        Token tokElse;

        ///
        /// @brief Assert annotation if the block is an assertion in a state
        /// script.
        ///
        /// @remark This is used to generate error messages and has no effect
        /// on how the block is compiled.
        ///
        Token tokAssert;

        ///
        /// @brief Stop annotation if the block is a state script stop
        /// condition.
        ///
        /// @remark This is used to generate error messages and has no effect
        /// on how the block is compiled.
        ///
        Token tokStop;
    };

    ///
    /// @brief Parse of a state.
    ///
    struct StateParse final
    {
        ///
        /// @brief State identifier.
        ///
        Token tokName;

        ///
        /// @brief Entry block parse.
        ///
        Ref<const StateMachineParse::BlockParse> entry;

        ///
        /// @brief Step block parse.
        ///
        Ref<const StateMachineParse::BlockParse> step;

        ///
        /// @brief Exit block parse.
        ///
        Ref<const StateMachineParse::BlockParse> exit;
    };

    ///
    /// @brief Parse of a state vector section element declaration.
    ///
    struct StateVectorElementParse final
    {
        ///
        /// @brief Element type identifier.
        ///
        Token tokType;

        ///
        /// @brief Element identifier.
        ///
        Token tokName;

        ///
        /// @brief Element alias identifier.
        ///
        Token tokAlias;

        ///
        /// @brief If element is read-only.
        ///
        bool readOnly;
    };

    ///
    /// @brief Parse of a local section element declaration.
    ///
    struct LocalElementParse final
    {
        ///
        /// @brief Element type identifier.
        ///
        Token tokType;

        ///
        /// @brief Element identifier.
        ///
        Token tokName;

        ///
        /// @brief Initial value expression parse.
        ///
        Ref<const ExpressionParse> initValExpr;

        ///
        /// @brief If element is read-only.
        ///
        bool readOnly;
    };

    ///
    /// @brief State vector element parses.
    ///
    Vec<StateMachineParse::StateVectorElementParse> svElems;

    ///
    /// @brief Local element parses.
    ///
    Vec<StateMachineParse::LocalElementParse> localElems;

    ///
    /// @brief State parses.
    ///
    Vec<StateMachineParse::StateParse> states;

    ///
    /// @brief If a state vector section was parsed.
    ///
    bool hasStateVectorSection;

    ///
    /// @brief If a local section was parsed.
    ///
    bool hasLocalSection;

private:

    friend class StateMachineParser;

    ///
    /// @brief Mutable block parse type used to construct the block tree. When
    /// parsing is done, the mutable tree is converted to a tree of BlockParse
    /// and returned to the user so that they cannot accidentally modify it.
    ///
    struct MutBlockParse final
    {
        ///
        /// @see StateMachineParse::BlockParse::guard
        ///
        Ref<const ExpressionParse> guard;

        ///
        /// @see StateMachineParse::BlockParse::action
        ///
        Ref<const StateMachineParse::ActionParse> action;

        ///
        /// @see StateMachineParse::BlockParse::ifBlock
        ///
        Ref<StateMachineParse::MutBlockParse> ifBlock;

        ///
        /// @see StateMachineParse::BlockParse::elseBlock
        ///
        Ref<StateMachineParse::MutBlockParse> elseBlock;

        ///
        /// @see StateMachineParse::BlockParse::next
        ///
        Ref<StateMachineParse::MutBlockParse> next;

        ///
        /// @see StateMachineParse::BlockParse::assert
        ///
        Ref<const ExpressionParse> assert;

        ///
        /// @see StateMachineParse::BlockParse::tokElse
        ///
        Token tokElse;

        ///
        /// @see StateMachineParse::BlockParse::tokAssert
        ///
        Token tokAssert;

        ///
        /// @see StateMachineParse::BlockParse::tokStop
        ///
        Token tokStop;

        ///
        /// @brief Recursively converts the tree rooted at this MutBlockParse
        /// to a tree of BlockParse.
        ///
        /// @param[in] kParse  Reference to store the converted node in.
        ///
        void toBlockParse(Ref<const StateMachineParse::BlockParse>& kParse);
    };

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kSvElems                State vector element parses.
    /// @param[in] kLocalElems             Local element parses.
    /// @param[in] kStates                 State parses.
    /// @param[in] kHasStateVectorSection  If a state vector section was parsed.
    /// @param[in] kHasLocalSection        If a local section was parsed.
    ///
    StateMachineParse(
        const Vec<StateMachineParse::StateVectorElementParse>& kSvElems,
        const Vec<StateMachineParse::LocalElementParse>& kLocalElems,
        const Vec<StateMachineParse::StateParse>& kStates,
        const bool kHasStateVectorSection,
        const bool kHasLocalSection);
};

///
/// @brief State machine parser.
///
class StateMachineParser final
{
public:

    ///
    /// @brief Parser entry point.
    ///
    /// @param[in]  kToks   Tokens to parse.
    /// @param[in]  kParse  On success, points to state machine parse.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @retval SUCCESS
    /// @retval E_SMP_NO_LAB     Expected a label.
    /// @retval E_SMP_LAB_DUPE   Duplicate label.
    /// @retval E_SMP_LAB        Invalid label.
    /// @retval E_SMP_ELEM_TYPE  Expected element type identifier.
    /// @retval E_SMP_ELEM_NAME  Expected element identifier.
    /// @retval E_SMP_LOC_OP     Expected assignment operator after local
    ///                          element identifier.
    /// @retval E_SMP_LOC_VAL    Expected local element initial value expression
    ///                          after assignment operator.
    /// @retval E_SMP_RO_MULT    Redundant read-only annotation.
    /// @retval E_SMP_ANNOT      Invalid annotation.
    /// @retval E_SMP_AL_MULT    Multiple alias annotations on same element.
    /// @retval E_SMP_ALIAS      Expected identifier after alias annotation.
    /// @retval E_SMP_SV_MULT    Multiple state vector sections.
    /// @retval E_SMP_LOC_MULT   Multiple local sections.
    /// @retval E_SMP_TOK        Unexpected token.
    /// @retval E_SMP_ACT_ELEM   Expected assignment operator element name.
    /// @retval E_SMP_ACT_OP     Unexpected operator after element name.
    /// @retval E_SMP_ACT_EXPR   Expected expression after assignment operator.
    /// @retval E_SMP_TR_DEST    Expected state identifier after transition.
    /// @retval E_SMP_TR_TOK     Unexpected token after transition keyword.
    /// @retval E_SMP_JUNK       Unexpected token after transition statement.
    /// @retval E_SMP_ACT_TOK    Unexpected token in action.
    /// @retval E_SMP_GUARD      Empty conditional.
    /// @retval E_SMP_BRACE      Unbalanced braces.
    ///
    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateMachineParse>& kParse,
                        ErrorInfo* const kErr);

    ///
    /// @brief Parses the local section.
    ///
    /// @remark Public for testing purposes only - do not access in production.
    ///
    /// @param[in]  kIt     Token iterator positioned at section token.
    /// @param[out] kParse  On success, contains parsed local elements.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @returns See StateMachineParser::parse().
    ///
    static Result parseLocalSection(
        TokenIterator& kIt,
        Vec<StateMachineParse::LocalElementParse>& kParse,
        ErrorInfo* const kErr);

    ///
    /// @brief Parses the state vector section.
    ///
    /// @remark Public for testing purposes only - do not access in production.
    ///
    /// @param[in]  kIt     Token iterator positioned at section token.
    /// @param[out] kParse  On success, contains parsed state vector elements.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @returns See StateMachineParser::parse().
    ///
    static Result parseStateVectorSection(
        TokenIterator& kIt,
        Vec<StateMachineParse::StateVectorElementParse>& kParse,
        ErrorInfo* const kErr);

    ///
    /// @brief Parses a state section.
    ///
    /// @remark Public for testing purposes only - do not access in production.
    ///
    /// @param[in]  kIt     Token iterator positioned at section token.
    /// @param[out] kParse  On success, contains parsed state.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @returns See StateMachineParser::parse().
    ///
    static Result parseStateSection(TokenIterator& kIt,
                                    StateMachineParse::StateParse& kParse,
                                    ErrorInfo* const kErr);

    StateMachineParser() = delete;

private:

    friend class StateScriptCompiler;

    ///
    /// @brief Parses the logic under a label in a state section.
    ///
    /// @param[in]  kIt     Token iterator positioned at first token after
    ///                     label.
    /// @param[out] kParse  On success, contains parsed block.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @returns See StateMachineParser::parse().
    ///
    static Result parseBlock(TokenIterator kIt,
                             Ref<const StateMachineParse::BlockParse>& kParse,
                             ErrorInfo* const kErr);

    ///
    /// @brief Parses an action.
    ///
    /// @param[in]  kIt     Token iterator positioned at first token in action.
    /// @param[out] kParse  On success, contains parsed action.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @returns See StateMachineParser::parse().
    ///
    static Result parseAction(
        TokenIterator kIt,
        Ref<const StateMachineParse::ActionParse>& kParse,
        ErrorInfo* const kErr);

    ///
    /// @brief Recursive helper for logic block parsing.
    ///
    /// @param[in]  kIt     Token iterator positioned at first token in block.
    /// @param[out] kParse  On success, contains parsed block.
    /// @param[out] kErr    On error, contains error info.
    ///
    /// @returns See StateMachineParser::parse().
    ///
    static Result parseBlockRec(TokenIterator kIt,
                                Ref<StateMachineParse::MutBlockParse>& kParse,
                                ErrorInfo* const kErr);

};

#endif
