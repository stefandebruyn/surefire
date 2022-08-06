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
/// @file  sf/config/StateMachineAutocoder.hpp
/// @brief Autocoder for a compiled state machine.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_MACHINE_AUTOCODER_HPP
#define SF_STATE_MACHINE_AUTOCODER_HPP

#include "sf/config/Autocode.hpp"
#include "sf/config/StateMachineCompiler.hpp"

///
/// @brief State machine autocoder.
///
class StateMachineAutocoder final
{
public:

    ///
    /// @brief Autocoding entry point.
    ///
    /// @param[in] kOs    Autocode output stream.
    /// @param[in] kName  Name of state machine (will be used for certain
    ///                   identifiers in autocode).
    /// @param[in] kAsm   State machine to autocode.
    ///
    /// @retval SUCCESS     Successfully generated autocode.
    /// @retval E_SMA_NULL  kAsm is null.
    ///
    static Result code(std::ostream& kOs,
                       const String kName,
                       const Ref<const StateMachineAssembly> kAsm);

    StateMachineAutocoder() = delete;

private:

    ///
    /// @brief Intermediate data used in autocoding.
    ///
    struct Workspace final
    {
        Ref<const StateMachineAssembly> smAsm; ///< State machine to autocode.
        Set<const IElement*> refElems;         ///< Elements referenced so far.
        U32 blockCnt;                          ///< Block count.
        U32 exprNodeCnt;                       ///< Expression node count.
        U32 stateCnt;                          ///< State count.
        U32 actCnt;                            ///< Action count.
        U32 statsCnt;                          ///< Expression stats count.
    };

    ///
    /// @brief Map of function addresses in the ExprOpFuncs namespace to their
    /// C++ identifiers.
    ///
    static const Map<const void*, String> opFuncIds;

    ///
    /// @brief Map of expression node types to C++ identifers of the
    /// corresponding class.
    ///
    static const Map<IExpression::NodeType, String> exprStatNodeIds;

    static String elemNameFromAddr(const IElement* const kAddr,
                                   StateMachineAutocoder::Workspace& kWs);

    static void codeLocalStateVector(Autocode& kAutocode,
                                     StateMachineAutocoder::Workspace& kWs);

    static String codeConstExprNode(const IExpression* const kNode,
                                    Autocode& kAutocode,
                                    StateMachineAutocoder::Workspace& kWs);

    static void codeElementLookup(Autocode& kAutocode,
                                  const IElement* const kElemObj,
                                  const TypeInfo& kElemTypeInfo,
                                  const String kElemName,
                                  StateMachineAutocoder::Workspace& kWs);

    static String codeElementExprNode(const IExpression* const kNode,
                                      Autocode& kAutocode,
                                      StateMachineAutocoder::Workspace& kWs);

    static String codeBinOpExprNode(const IExpression* const kNode,
                                    Autocode& kAutocode,
                                    StateMachineAutocoder::Workspace& kWs);

    static String codeUnaryOpExprNode(const IExpression* const kNode,
                                      Autocode& kAutocode,
                                      StateMachineAutocoder::Workspace& kWs);

    static String codeExprStatsNode(const IExpression* const kNode,
                                    Autocode& kAutocode,
                                    StateMachineAutocoder::Workspace& kWs);

    static String codeExpression(const IExpression* const kExpr,
                                 Autocode& kAutocode,
                                 StateMachineAutocoder::Workspace& kWs);

    static String codeAction(const IAction* const kAction,
                             Autocode& kAutocode,
                             StateMachineAutocoder::Workspace& kWs);

    static String codeBlock(const StateMachine::Block* const kBlock,
                            Autocode& kAutocode,
                            StateMachineAutocoder::Workspace& kWs);

    static void codeState(const StateMachine::StateConfig* const kState,
                          Autocode& kAutocode,
                          StateMachineAutocoder::Workspace& kWs);
};

#endif
