////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
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
/// @file  sf/config/StateMachineCompiler.hpp
/// @brief State machine DSL compiler.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_MACHINE_COMPILER_HPP
#define SF_STATE_MACHINE_COMPILER_HPP

#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/StateMachineParser.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/StateVector.hpp"

///
/// @brief Compiled state machine.
///
class StateMachineAssembly final
{
public:

    ///
    /// @brief Gets the underlying StateMachine object.
    ///
    /// @returns State machine.
    ///
    StateMachine& get() const;

    ///
    /// @brief Gets the config used to configure the state machine.
    ///
    /// @remark This is mostly for testing purposes and should not be accessed
    /// in production.
    ///
    /// @returns State machine config.
    ///
    StateMachine::Config config() const;

    ///
    /// @brief Gets the state vector of local state machine variables.
    ///
    /// @remark This is mostly for testing purposes and should not be accessed
    /// in production.
    ///
    /// @returns Local state vector.
    ///
    StateVector& localStateVector() const;

    ///
    /// @brief Gets the parse used to compile the state machine.
    ///
    /// @remark This is mostly for testing purposes and should not be accessed
    /// in production.
    ///
    /// @returns State machine parse.
    ///
    Ref<const StateMachineParse> parse() const;

private:

    friend class StateMachineCompiler;

    friend class StateMachineAutocoder;

    friend class StateScriptCompiler;

    friend class StateScriptAssembly;

    ///
    /// @brief Set of data that represents the state machine.
    ///
    struct Workspace final
    {
        ///
        /// @brief Map of variable identifiers to state vector elements.
        ///
        Map<String, IElement*> elems;

        ///
        /// @brief Map of state identifiers to state IDs.
        ///
        Map<String, U32> stateIds;

        ///
        /// @brief Set of read-only elements.
        ///
        Set<String> readOnlyElems;

        ///
        /// @brief State vector used by state machine.
        ///
        Ref<const StateVectorAssembly> svAsm;

        ///
        /// @brief State vector of local state machine variables.
        ///
        Ref<const StateVectorAssembly> localSvAsm;

        ///
        /// @brief Expressions in the state machine.
        ///
        Vec<Ref<const ExpressionAssembly>> exprAsms;

        ///
        /// @brief State configs in the state machine.
        ///
        Ref<Vec<StateMachine::StateConfig>> stateConfigs;

        ///
        /// @brief Blocks in the state machine.
        ///
        Vec<Ref<StateMachine::Block>> blocks;

        ///
        /// @brief Actions in the state machine.
        ///
        Vec<Ref<IAction>> actions;

        ///
        /// @brief Expression stats in the state machine.
        ///
        Ref<Vec<IExpressionStats*>> exprStatArr;

        ///
        /// @brief Main state machine object.
        ///
        Ref<StateMachine> sm;

        ///
        /// @brief State machine config.
        ///
        StateMachine::Config smConfig;

        ///
        /// @brief Parse used to compile the state machine.
        ///
        Ref<const StateMachineParse> smParse;

        ///
        /// @brief If the state machine assembly has been raked.
        ///
        bool raked;
    };

    ///
    /// @brief Workspace.
    ///
    const StateMachineAssembly::Workspace mWs;

    ///
    /// @brief Contructor.
    ///
    /// @param[in] kWs  Workspace.
    ///
    StateMachineAssembly(const StateMachineAssembly::Workspace& kWs);
};

///
/// @brief State machine compiler.
///
class StateMachineCompiler final
{
public:

    ///
    /// @brief When passed as kInitState to compile(), the state machine initial
    /// state will be the first state defined in the state machine config.
    ///
    static const String FIRST_STATE;

    ///
    /// @brief Compiler entry point, taking a path to a state machine config
    /// file.
    ///
    /// @param[in]  kFilePath   Path to state machine config file.
    /// @param[in]  kSvAsm      State vector for state machine to use.
    /// @param[out] kAsm        On success, points to compiled state machine.
    /// @param[out] kErr        On error, if non-null, contains error info.
    /// @param[in]  kInitState  Name of state machine initial state, or
    ///                         FIRST_STATE for the first defined state.
    /// @param[in]  kRake       If the state machine assembly should be raked,
    ///                         meaning all data not needed to run the state
    ///                         machine is deallocated. This should always be
    ///                         true in production.
    ///
    /// @retval SUCCESS           Successfully compiled state machine.
    /// @retval E_SMC_FILE        Failed to open state machine config file.
    /// @retval E_SMC_NULL        State machine parse is null.
    /// @retval E_SMC_INIT        Unknown initial state.
    /// @retval E_SMC_SV_ELEM     Unknown state vector element.
    /// @retval E_SMC_TYPE        Invalid element type.
    /// @retval E_SMC_TYPE_MISM   State vector element type mismatch.
    /// @retval E_SMC_ELEM_DUPE   Element declared twice.
    /// @retval E_SMC_NO_G        No global time element aliased.
    /// @retval E_SMC_NO_S        No state element aliased.
    /// @retval E_SMC_LOC_SV_REF  Non-local element referenced in local section.
    /// @retval E_SMC_UBI         Local element used before initialization.
    /// @retval E_SMC_ASG_ELEM    Unknown element on LHS of assignment.
    /// @retval E_SMC_ELEM_RO     Tried to write a read-only element.
    /// @retval E_SMC_TR_EXIT     Illegal transition in exit block.
    /// @retval E_SMC_STATE       Unknown destination state.
    /// @retval E_SMC_ASSERT      Illegal assertion annotation in state machine.
    /// @retval E_SMC_STOP        Illegal stop annotation in state machine.
    /// @retval E_SMC_RSVD        State name is reserved.
    ///
    static Result compile(const String kFilePath,
                          const Ref<const StateVectorAssembly> kSvAsm,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr,
                          const String kInitState = FIRST_STATE,
                          const bool kRake = true);

    ///
    /// @brief Compiler entry point, taking an input stream with the state
    /// machine config file.
    ///
    /// @see StateMachineCompiler::compile(String, ...)
    ///
    static Result compile(std::istream& kIs,
                          const Ref<const StateVectorAssembly> kSvAsm,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr,
                          const String kInitState = FIRST_STATE,
                          const bool kRake = true);

    ///
    /// @brief Compiler entry point, taking a state machine parse.
    ///
    /// @see StateMachineCompiler::compile(String, ...)
    ///
    static Result compile(const Ref<const StateMachineParse> kParse,
                          const Ref<const StateVectorAssembly> kSvAsm,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr,
                          const String kInitState = FIRST_STATE,
                          const bool kRake = true);

    StateMachineCompiler() = delete;

private:

    friend class StateScriptCompiler;

    ///
    /// @brief Gets whether a state name is reserved.
    ///
    /// @param[in] kTokSection  State name to check.
    ///
    /// @returns If state name is reserved.
    ///
    static bool stateNameReserved(const Token& kTokSection);

    ///
    /// @brief Validates the state vector section.
    ///
    /// @param[in]  kParse  State machine parse.
    /// @param[in]  kWs     Compiler workspace.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result checkStateVector(const Ref<const StateMachineParse> kParse,
                                   StateMachineAssembly::Workspace& kWs,
                                   ErrorInfo* const kErr);

    ///
    /// @brief Compiles the local state vector section.
    ///
    /// @param[in]  kParse  State machine parse.
    /// @param[in]  kWs     Compiler workspace.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result compileLocalStateVector(
        const Ref<const StateMachineParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    ///
    /// @brief Recursively validates an initialization expression for a local
    /// element.
    ///
    /// @param[in]  kInitElem  Parse of element being validated.
    /// @param[in]  kExpr      Root of expression tree to validate.
    /// @param[in]  kWs        Compiler workspace.
    /// @param[out] kErr       On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result checkLocalElemInitExprs(
        const StateMachineParse::LocalElementParse& kInitElem,
        const Ref<const ExpressionParse> kExpr,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    ///
    /// @brief Evaluates local element initialization expressions and assigns
    /// initial values.
    ///
    /// @param[in]  kParse  State machine parse.
    /// @param[in]  kWs     Compiler workspace.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result initLocalElementValues(
        const Ref<const StateMachineParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    ///
    /// @brief Compiles an assignment action.
    ///
    /// @remark This function is devoid of StateMachineCompiler-specific types
    /// like StateMachineCompiler::Workspace so that the function may be reused
    /// by friend class StateScriptCompiler.
    ///
    /// @param[in]  kParse          Action parse to compile.
    /// @param[in]  kBindings       Element symbol table.
    /// @param[in]  kReadOnlyElems  Set of read-only element names.
    /// @param[out] kAction         On success, points to compiled action.
    /// @param[in]  kRhsAsm         RHS of assignment.
    /// @param[out] kErr            On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result compileAssignmentAction(
        const Ref<const StateMachineParse::ActionParse> kParse,
        const Map<String, IElement*>& kBindings,
        const Set<String>& kReadOnlyElems,
        Ref<IAction>& kAction,
        Ref<const ExpressionAssembly>& kRhsAsm,
        ErrorInfo* const kErr);

    ///
    /// @brief Compiles an action.
    ///
    /// @param[in]  kParse        Action parse to compile.
    /// @param[in]  kWs           Compiler workspace.
    /// @param[in]  kInExitLabel  If action is in an exit block.
    /// @param[out] kAction       On success, points to compiled action.
    /// @param[out] kErr          On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result compileAction(
        const Ref<const StateMachineParse::ActionParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        const bool kInExitLabel,
        Ref<IAction>& kAction,
        ErrorInfo* const kErr);

    ///
    /// @brief Recursively compiles a block.
    ///
    /// @param[in]  kParse        Block parse to compile.
    /// @param[in]  kWs           Compiler workspace.
    /// @param[in]  kInExitLabel  If action is in an exit block.
    /// @param[out] kAction       On success, points to compiled action.
    /// @param[out] kErr          On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result compileBlock(
        const Ref<const StateMachineParse::BlockParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        const bool kInExitLabel,
        Ref<StateMachine::Block>& kBlock,
        ErrorInfo* const kErr);

    ///
    /// @brief Compiles a state.
    ///
    /// @param[in]  kParse  State parse to compile.
    /// @param[in]  kWs     Compiler workspace.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @returns See StateMachineCompiler::compile().
    ///
    static Result compileState(const StateMachineParse::StateParse& kParse,
                               StateMachineAssembly::Workspace& kWs,
                               ErrorInfo* const kErr);
};

#endif
