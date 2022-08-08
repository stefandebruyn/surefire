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
/// @file  sf/config/StateScriptCompiler.hpp
/// @brief State script compiler.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_SCRIPT_COMPILER_HPP
#define SF_STATE_SCRIPT_COMPILER_HPP

#include <iostream>

#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateScriptParser.hpp"

///
/// @brief Compiled state script.
///
class StateScriptAssembly final
{
public:

    ///
    /// @brief Represents the results of a state script run.
    ///
    struct Report final
    {
        bool pass;   ///< If the state script passed.
        U64 steps;   ///< Number of state machine steps.
        U64 asserts; ///< Number of passed asserts.
        String text; ///< Prettified report text for printing.
    };

    ///
    /// @brief Runs the state script.
    ///
    /// @remark If state script stop conditions are unreachable, this method
    /// will never return.
    ///
    /// @param[out] kTokInfo  On assertion failure, contains error info.
    /// @param[out] kReport   On success, contains state script results.
    ///
    /// @retval SUCCESS     Successfully ran state script. This does not
    ///                     necessarily mean the script passed.
    /// @retval E_SSC_OVFL  Global time overflowed. This probably means the
    ///                     specified delta T is too large.
    ///
    Result run(ErrorInfo& kTokInfo, StateScriptAssembly::Report& kReport);

private:

    friend class StateScriptCompiler;

    ///
    /// @brief Input that runs before a state script step.
    ///
    struct Input final
    {
        ///
        /// @brief Input conditional.
        ///
        IExprNode<bool>* guard;

        ///
        /// @brief Input action. This is always an assignment action.
        ///
        Ref<IAction> action;
    };

    ///
    /// @brief Assertion that runs after a state script step.
    ///
    struct Assert final
    {
        ///
        /// @brief Assertion conditional.
        ///
        IExprNode<bool>* guard;

        ///
        /// @brief Assertion statement.
        ///
        IExprNode<bool>* assert;

        ///
        /// @brief Assertion annotation.
        ///
        /// @remark This is used to generate error messages.
        ///
        Token tokAssert;
    };

    ///
    /// @brief Section of a state script.
    ///
    struct Section final
    {
        ///
        /// @brief ID of state that section runs in, or StateMachine::NO_STATE
        /// if the section runs in all states.
        ///
        U32 stateId;

        ///
        /// @brief Section inputs.
        ///
        Vec<StateScriptAssembly::Input> inputs;

        ///
        /// @brief Section assertions.
        ///
        Vec<StateScriptAssembly::Assert> asserts;
    };

    ///
    /// @brief State script config.
    ///
    struct Config final
    {
        U64 deltaT;    ///< Delta T in global time unit.
        U32 initState; ///< ID of initial state.
    };

    ///
    /// @brief State script sections.
    ///
    Vec<StateScriptAssembly::Section> mSections;

    ///
    /// @brief State machine being run in the state script.
    ///
    const Ref<const StateMachineAssembly> mSmAsm;

    ///
    /// @brief Expressions used in state script inputs and assertions.
    ///
    Vec<Ref<const ExpressionAssembly>> mExprAsms;

    ///
    /// @brief State script config.
    ///
    StateScriptAssembly::Config mConfig;

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kSections  State script sections.
    /// @param[in] kSmAsm     State machine being run in the state script.
    /// @param[in] kExprAsms  Expressions used in state script inputs and
    ///                       assertions.
    /// @param[in] kConfig    State script config.
    ///
    StateScriptAssembly(const Vec<StateScriptAssembly::Section>& kSections,
                        const Ref<const StateMachineAssembly> kSmAsm,
                        const Vec<Ref<const ExpressionAssembly>> kExprAsms,
                        const StateScriptAssembly::Config& kConfig);

    ///
    /// @brief Pretty-prints the current state vector, including the state
    /// machine local state vector.
    ///
    /// @param[in] kOs  Output stream to print to.
    ///
    /// @retval SUCCESS  Always succeeds (unless an assertion fails).
    ///
    Result printStateVector(std::ostream& kOs);
};

///
/// @brief State script compiler.
///
class StateScriptCompiler final
{
public:

    ///
    /// @brief Compiler entry point, taking a path to the state script file.
    ///
    /// @param[in]  kFilePath  Path to state script file.
    /// @param[in]  kSmAsm     State machine to run in state script. The state
    ///                        machine assembly must not be raked, as the extra
    ///                        data produced during state machine compilation is
    ///                        needed to run the state script.
    /// @param[out] kAsm       On success, points to compiled state script.
    /// @param[out] kErr       On error, contains error info.
    ///
    /// @retval SUCCESS      Successfully compiled state script.
    /// @retval E_SSC_FILE   Failed to open state script file.
    /// @retval E_SSC_NULL   State script parse or state machine assembly null.
    /// @retval E_SSC_RAKE   State machine assembly was raked.
    /// @retval E_SSC_STATE  Unknown state.
    /// @retval E_SSC_DUPE   Duplicate state section.
    /// @retval E_SSC_GUARD  Unguarded statement.
    /// @retval E_SSC_ELSE   Illegal else branch.
    /// @retval E_SSC_NEST   Illegal nested block.
    /// @retval E_SSC_UNRCH  Unreachable statement after stop annotation.
    /// @retval E_SSC_STOP   No stop annotation.
    /// @retval E_SSC_DT     Delta T was not specified or is invalid.
    ///
    static Result compile(const String kFilePath,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Ref<StateScriptAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ///
    /// @brief Compiler entry point, taking an input stream of the state script.
    ///
    /// @see StateScriptCompiler::compile(String, ...)
    ///
    static Result compile(std::istream& kIs,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Ref<StateScriptAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ///
    /// @brief Compiler entry point, taking a state script parse.
    ///
    /// @see StateScriptCompiler::compile(String, ...)
    ///
    static Result compile(const Ref<const StateScriptParse> kParse,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Ref<StateScriptAssembly>& kAsm,
                          ErrorInfo* const kErr);

    StateScriptCompiler() = delete;

private:

    ///
    /// @brief Compiles the state script options.
    ///
    /// @param[in]  kParse   Options parse.
    /// @param[in]  kSmAsm   State machine assembly.
    /// @param[in]  kConfig  On success, contains compiled options.
    /// @param[out] kErr     On error, contains error info.
    ///
    /// @returns See StateScriptCompiler::compile().
    ///
    static Result compileOptions(const StateScriptParse::Config& kParse,
                                 const Ref<const StateMachineAssembly> kSmAsm,
                                 StateScriptAssembly::Config& kConfig,
                                 ErrorInfo* const kErr);
};

#endif
