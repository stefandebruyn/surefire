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
/// @file  sf/config/StateScriptParser.hpp
/// @brief State script parser.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_SCRIPT_PARSER_HPP
#define SF_STATE_SCRIPT_PARSER_HPP

#include "sf/config/StateMachineParser.hpp"

///
/// @brief Parse of a state script.
///
class StateScriptParse final
{
public:

    ///
    /// @brief Parse of a state script section.
    ///
    struct SectionParse final
    {
        ///
        /// @brief Section name.
        ///
        Token tokName;

        ///
        /// @brief Section block parse.
        ///
        Ref<const StateMachineParse::BlockParse> block;
    };

    ///
    /// @brief Parse of a state script config section.
    ///
    struct Config final
    {
        ///
        /// @brief Delta T option identifier token.
        ///
        /// @remark This is used to generate error messages.
        ///
        Token tokDeltaT;
        
        ///
        /// @brief Initial state identifier.
        ///
        Token tokInitState;

        ///
        /// @brief Delta T value.
        ///
        U64 deltaT;
    };

    ///
    /// @brief State script sections.
    ///
    Vec<StateScriptParse::SectionParse> sections;

    ///
    /// @brief State script config.
    ///
    StateScriptParse::Config config;

private:

    friend class StateScriptParser;

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kSections  State script sections.
    /// @param[in] kConfig    State script config.
    ///
    StateScriptParse(const Vec<StateScriptParse::SectionParse>& kSections,
                     const StateScriptParse::Config& kConfig);
};

///
/// @brief State script parser.
///
class StateScriptParser final
{
public:

    ///
    /// @brief Parser entry point.
    ///
    /// @param[in]  kToks   Tokens to parse.
    /// @param[out] kParse  On success, points to state script parse.
    /// @param[out] kErr    On error, if non-null, contains error info.
    ///
    /// @retval SUCCESS       Successfully parsed state script.
    /// @retval E_SSP_SEC     Expected a section.
    /// @retval E_SSP_DT      Expected constant after delta T option.
    /// @retval E_SSP_STATE   Expected state name after initial state option.
    /// @retval E_SSP_CONFIG  Unknown config option.
    ///
    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateScriptParse>& kParse,
                        ErrorInfo* const kErr);

    StateScriptParser() = delete;
};

#endif
