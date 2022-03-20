// #include <fstream>

// #include "sf/config/ConfigUtil.hpp"
// #include "sf/config/ExpressionCompiler.hpp"
// #include "sf/config/StateMachineCompiler.hpp"
// #include "sf/config/StateVectorCompiler.hpp"
// #include "sf/core/Assert.hpp"

// /////////////////////////////////// Private ////////////////////////////////////

// namespace
// {

// const char* const errText = "state machine config error";

// const String elemStateTimeName = "T";

// const String elemGlobalTimeName = "G";

// const String elemStateName = "S";

// struct CompilerState final
// {
//     Map<String, IElement*> elems;
//     Map<String, U32> stateIds;
//     Ref<const StateVectorCompiler::Assembly> localSvAsm;
//     StateVector* localSv;
//     Vec<StateMachine::StateConfig> states;
//     Vec<Ref<const ExpressionCompiler::Assembly>> exprs;
//     Set<String> readOnlyElems;
// };

// void deleteBlock(StateMachine::Block* const kBlock)
// {
//     // Base case: null block.
//     if (kBlock == nullptr)
//     {
//         return;
//     }

//     // Delete linked blocks.
//     deleteBlock(kBlock->ifBlock);
//     kBlock->ifBlock = nullptr;
//     deleteBlock(kBlock->elseBlock);
//     kBlock->elseBlock = nullptr;
//     deleteBlock(kBlock->next);
//     kBlock->next = nullptr;

//     // Delete block action and finally the block itself.
//     delete kBlock->action;
//     kBlock->action = nullptr;
//     delete kBlock;

//     // Note: the block guard is not deleted since expressions in the state
//     // machine are owned by expression assemblies.
// }

// Result checkStateVector(const StateMachineParser::Parse& kParse,
//                         const StateVector& kSv,
//                         CompilerState& kCompState,
//                         ErrorInfo* const kErr)
// {
//     for (const StateMachineParser::StateVectorElementParse& elem :
//          kParse.svElems)
//     {
//         // Get element object from state vector.
//         IElement* elemObj = nullptr;
//         if (kSv.getIElement(elem.tokName.str.c_str(), elemObj) != SUCCESS)
//         {
//             // Element does not exist in state vector.
//             ConfigUtil::setError(kErr, elem.tokName, errText,
//                                  "element `" + elem.tokName.str + "` does not "
//                                  "exist in state vector");
//             return E_SMC_SV_ELEM;
//         }

//         // Look up element type as configured in the state machine.
//         auto smTypeInfoIt = ElementTypeInfo::fromName.find(elem.tokType.str);
//         if (smTypeInfoIt == ElementTypeInfo::fromName.end())
//         {
//             // Unknown type.
//             ConfigUtil::setError(kErr, elem.tokType, errText,
//                                  "unknown type `" + elem.tokType.str + "`");
//             return E_SMC_TYPE;
//         }
//         const ElementTypeInfo& smTypeInfo = (*smTypeInfoIt).second;

//         // Look up element type info as configured in the actual state vector.
//         auto typeInfoIt = ElementTypeInfo::fromEnum.find(elemObj->type());
//         // Assert that lookup succeeds since valid element types are guaranteed
//         // by the state vector.
//         SF_ASSERT(typeInfoIt != ElementTypeInfo::fromEnum.end());
//         const ElementTypeInfo& typeInfo = (*typeInfoIt).second;

//         // Check that element has the same type in the state vector and state
//         // machine.
//         if (typeInfo.enumVal != smTypeInfo.enumVal)
//         {
//             std::stringstream ss;
//             ss << "element `" << elem.tokName.str << "` is type "
//                << typeInfo.name << " in the state vector but type "
//                << smTypeInfo.name << " here";
//             ConfigUtil::setError(kErr, elem.tokType, errText, ss.str());
//             return E_SMC_TYPE_MISM;
//         }

//         // Check that element does not appear twice in the state machine.
//         if (kCompState.elems.find(elem.tokName.str) != kCompState.elems.end())
//         {
//             ConfigUtil::setError(kErr, elem.tokName, errText,
//                                  "element `" + elem.tokName.str + "` is listed "
//                                  "more than once");
//             return E_SMC_ELEM_DUPE;
//         }

//         // Add element to the symbol table.
//         SF_ASSERT(elemObj != nullptr);
//         kCompState.elems[elem.tokName.str] = elemObj;

//         // Make a copy of the element read-only flag. The read-onlyness may
//         // change in special cases.
//         bool elemReadOnly = elem.readOnly;

//         // Check that global time element is the correct type.
//         if ((elem.tokName.str == elemGlobalTimeName)
//             || (elem.alias == elemGlobalTimeName))
//         {
//             // Global time element is automatically read-only.
//             elemReadOnly = true;

//             if (smTypeInfo.enumVal != ElementType::UINT64)
//             {
//                 // Global time element is not U64.
//                 std::stringstream ss;
//                 ss << "`" << elemGlobalTimeName << "` must be type U64 ("
//                    << elem.tokType.str << " here)";
//                 ConfigUtil::setError(kErr, elem.tokName, errText, ss.str());
//                 return E_SMC_G_TYPE;
//             }
//         }

//         // Check that state element is the correct type.
//         if ((elem.tokName.str == elemStateName)
//             || (elem.alias == elemStateName))
//         {
//             // State element is automatically read-only.
//             elemReadOnly = true;

//             if (smTypeInfo.enumVal != ElementType::UINT32)
//             {
//                 // State element is not U32.
//                 std::stringstream ss;
//                 ss << "`" << elemStateName << "` must be type U32 ("
//                    << elem.tokType.str << " here)";
//                 ConfigUtil::setError(kErr, elem.tokName, errText, ss.str());
//                 return E_SMC_S_TYPE;
//             }
//         }

//         // If the element is aliased, add the alias to the symbol table too.
//         if (elem.alias.size() > 0)
//         {
//             SF_ASSERT(elemObj != nullptr);
//             kCompState.elems[elem.alias] = elemObj;
//         }

//         // If element is read-only, add its name and alias to read-only set.
//         if (elemReadOnly)
//         {
//             kCompState.readOnlyElems.insert(elem.tokName.str);
//             if (elem.alias.size() > 0)
//             {
//                 kCompState.readOnlyElems.insert(elem.alias);
//             }
//         }
//     }

//     if (kCompState.elems.find(elemGlobalTimeName) == kCompState.elems.end())
//     {
//         // No global time element provided.
//         if (kErr != nullptr)
//         {
//             kErr->text = errText;
//             kErr->subtext = "no global time element aliased to `"
//                             + elemGlobalTimeName + "`";
//         }
//         return E_SMC_NO_G;
//     }

//     if (kCompState.elems.find(elemStateName) == kCompState.elems.end())
//     {
//         // No state element provided.
//         if (kErr != nullptr)
//         {
//             kErr->text = errText;
//             kErr->subtext = "no state element aliased to `" + elemStateName
//                             + "`";
//         }
//         return E_SMC_NO_S;
//     }

//     return SUCCESS;
// }

// Result compileLocalStateVector(const StateMachineParser::Parse& kParse,
//                                CompilerState& kCompState,
//                                ErrorInfo* const kErr)
// {
//     // To compile the local state vector, we'll build a state vector parse using
//     // tokens from the state machine parse and then compile it using the state
//     // vector compiler. The local state vector has all elements in a single
//     // region named "LOCAL".
//     StateVectorParser::Parse localSvParse =
//     {
//         // Regions
//         {
//             // Local region
//             {
//                 {Token::SECTION, "[LOCAL]", -1, -1},
//                 "LOCAL",
//                 {}
//             }
//         }
//     };

//     // Add built-in state machine elements.
//     localSvParse.regions[0].elems.push_back(
//         {
//             {Token::IDENTIFIER, "U64", -1, -1},
//             {Token::IDENTIFIER, elemStateTimeName, -1, -1}
//         });

//     // State time element is automatically read-only.
//     kCompState.readOnlyElems.insert(elemStateTimeName);

//     for (U32 i = 0; i < kParse.localElems.size(); ++i)
//     {
//         const StateMachineParser::LocalElementParse& elem =
//             kParse.localElems[i];

//         // Check for name uniqueness against state vector elements and aliases.
//         // Uniqueness against local elements will be checked by the state vector
//         // compiler later on.
//         for (const StateMachineParser::StateVectorElementParse& svElem :
//              kParse.svElems)
//         {
//             if ((elem.tokName.str == svElem.tokName.str)
//                 || ((svElem.alias.size() > 0)
//                     && (elem.tokName.str == svElem.alias)))
//             {
//                 std::stringstream ss;
//                 ss << "reuse of element name `" << elem.tokName.str
//                    << "` (previously used on line " << svElem.tokName.lineNum
//                    << ")";
//                 ConfigUtil::setError(kErr, elem.tokName, errText, ss.str());
//                 return E_SMC_ELEM_DUPE;
//             }
//         }

//         // Add element to local state vector parse.
//         localSvParse.regions[0].elems.push_back({elem.tokType, elem.tokName});

//         // If element is read-only, add to read-only set.
//         if (elem.readOnly)
//         {
//             kCompState.readOnlyElems.insert(elem.tokName.str);
//         }
//     }

//     // Compile the local state vector. Since the local state vector parse is at
//     // least syntatically correct, there are very few potential errors that the
//     // state vector compiler can generate here.
//     Result res = StateVectorCompiler::compile(localSvParse,
//                                               kCompState.localSvAsm,
//                                               kErr);
//     if (res != SUCCESS)
//     {
//         // Overwrite error text set by state vector compiler for consistent
//         // error messages from the state machine compiler.
//         if (kErr != nullptr)
//         {
//             kErr->text = errText;
//         }
//         return res;
//     }

//     // Configure the local state vector from the compiled config. Assert that
//     // this succeeds since the config is known to be valid at this point.
//     kCompState.localSv = new StateVector();
//     res = StateVector::create(kCompState.localSvAsm->getConfig(),
//                               *kCompState.localSv);
//     SF_ASSERT(res == SUCCESS);

//     // Look up each element object in the local state vector and add it to the
//     // element symbol table.
//     for (const StateVectorParser::ElementParse& elem :
//          localSvParse.regions[0].elems)
//     {
//         IElement* elemObj = nullptr;
//         res = kCompState.localSv->getIElement(elem.tokName.str.c_str(),
//                                               elemObj);
//         // Assert that element lookup succeeds since we configured the local
//         // state vector in this function and know the element exists.
//         SF_ASSERT(res == SUCCESS);
//         SF_ASSERT(elemObj != nullptr);
//         kCompState.elems[elem.tokName.str] = elemObj;
//     }

//     return SUCCESS;
// }

// Result initLocalElementValues(const StateMachineParser::Parse& kParse,
//                               CompilerState& kCompState,
//                               ErrorInfo* const kErr)
// {
//     (void) kParse;
//     (void) kCompState;
//     (void) kErr; // rm later

//     return SUCCESS;
// }

// Result compileAction(const StateMachineParser::ActionParse& kParse,
//                      const StateVector& kSv,
//                      CompilerState& kCompState,
//                      const bool kInExitLabel,
//                      IAction*& kAction,
//                      ErrorInfo* const kErr)
// {
//     Result res = SUCCESS;

//     if (kParse.lhs != nullptr)
//     {
//         // Compile assignment action.

//         // Look up RHS element.
//         auto elemIt = kCompState.elems.find(kParse.tokRhs.str);
//         if (elemIt == kCompState.elems.end())
//         {
//             // Unknown element.
//             ConfigUtil::setError(kErr, kParse.tokRhs, errText,
//                                  "unknown element `" + kParse.tokRhs.str + "`");
//             return E_SMC_ASG_ELEM;
//         }
//         IElement* const elemObj = (*elemIt).second;

//         // Check that RHS element is not read-only. This includes elements
//         // marked read-only by the user and reserved elements.
//         if ((kCompState.readOnlyElems.find(kParse.tokRhs.str) !=
//              kCompState.readOnlyElems.end())
//             || (kParse.tokRhs.str == elemGlobalTimeName)
//             || (kParse.tokRhs.str == elemStateTimeName)
//             || (kParse.tokRhs.str == elemStateName))
//         {
//             ConfigUtil::setError(kErr, kParse.tokRhs, errText,
//                                  "element `" + kParse.tokRhs.str + "` is "
//                                  "read-only");
//             return E_SMC_ELEM_RO;
//         }

//         // Compile LHS expression.
//         Ref<const ExpressionCompiler::Assembly> lhsAsm;
//         res = ExpressionCompiler::compile(kParse.lhs,
//                                           {&kSv, kCompState.localSv},
//                                           elemObj->type(),
//                                           lhsAsm,
//                                           kErr);
//         if (res != SUCCESS)
//         {
//             // Override error text set by expression compiler for consistent
//             // state machine compiler error messages.
//             if (kErr != nullptr)
//             {
//                 kErr->text = errText;
//             }

//             return res;
//         }

//         // Track compiled expression in compilation state.
//         kCompState.exprs.push_back(lhsAsm);

//         // Create assignment action based on element type. The element object
//         // and LHS root nodes are narrowed to template instantiations that match
//         // the element type. These casts are guaranteed correct in this context
//         // by the element and expression compiler implementations.
//         switch (elemObj->type())
//         {
//             case ElementType::INT8:
//                 kAction = new AssignmentAction<I8>(
//                     *static_cast<Element<I8>*>(elemObj),
//                     *static_cast<IExprNode<I8>*>(lhsAsm->root()));
//                 break;

//             case ElementType::INT16:
//                 kAction = new AssignmentAction<I16>(
//                     *static_cast<Element<I16>*>(elemObj),
//                     *static_cast<IExprNode<I16>*>(lhsAsm->root()));
//                 break;

//             case ElementType::INT32:
//                 kAction = new AssignmentAction<I32>(
//                     *static_cast<Element<I32>*>(elemObj),
//                     *static_cast<IExprNode<I32>*>(lhsAsm->root()));
//                 break;

//             case ElementType::INT64:
//                 kAction = new AssignmentAction<I64>(
//                     *static_cast<Element<I64>*>(elemObj),
//                     *static_cast<IExprNode<I64>*>(lhsAsm->root()));
//                 break;

//             case ElementType::UINT8:
//                 kAction = new AssignmentAction<U8>(
//                     *static_cast<Element<U8>*>(elemObj),
//                     *static_cast<IExprNode<U8>*>(lhsAsm->root()));
//                 break;

//             case ElementType::UINT16:
//                 kAction = new AssignmentAction<U16>(
//                     *static_cast<Element<U16>*>(elemObj),
//                     *static_cast<IExprNode<U16>*>(lhsAsm->root()));
//                 break;

//             case ElementType::UINT32:
//                 kAction = new AssignmentAction<U32>(
//                     *static_cast<Element<U32>*>(elemObj),
//                     *static_cast<IExprNode<U32>*>(lhsAsm->root()));
//                 break;

//             case ElementType::UINT64:
//                 kAction = new AssignmentAction<U64>(
//                     *static_cast<Element<U64>*>(elemObj),
//                     *static_cast<IExprNode<U64>*>(lhsAsm->root()));
//                 break;

//             case ElementType::FLOAT32:
//                 kAction = new AssignmentAction<F32>(
//                     *static_cast<Element<F32>*>(elemObj),
//                     *static_cast<IExprNode<F32>*>(lhsAsm->root()));
//                 break;

//             case ElementType::FLOAT64:
//                 kAction = new AssignmentAction<F64>(
//                     *static_cast<Element<F64>*>(elemObj),
//                     *static_cast<IExprNode<F64>*>(lhsAsm->root()));
//                 break;

//             case ElementType::BOOL:
//                 kAction = new AssignmentAction<bool>(
//                     *static_cast<Element<bool>*>(elemObj),
//                     *static_cast<IExprNode<bool>*>(lhsAsm->root()));
//                 break;

//             default:
//                 SF_ASSERT(false);
//         }
//     }
//     else
//     {
//         // Compile transition action.

//         if (kInExitLabel)
//         {
//             // Illegal transition in exit label.
//             ConfigUtil::setError(kErr, kParse.tokDestState, errText,
//                                  "illegal transition in exit label");
//             return E_SMC_TR_EXIT;
//         }

//         auto stateIdIt = kCompState.stateIds.find(kParse.tokDestState.str);
//         if (stateIdIt == kCompState.stateIds.end())
//         {
//             // Unknown destination state.
//             ConfigUtil::setError(kErr, kParse.tokDestState, errText,
//                                  "unknown state `" + kParse.tokDestState.str
//                                  + "`");
//             return E_SMC_STATE;
//         }
//         const U32 destState = (*stateIdIt).second;
//         kAction = new TransitionAction(destState);
//     }

//     return SUCCESS;
// }

// Result compileBlock(const StateMachineParser::BlockParse& kParse,
//                     const StateVector& kSv,
//                     CompilerState& kCompState,
//                     const bool kInExitLabel,
//                     StateMachine::Block*& kBlock,
//                     ErrorInfo* const kErr)
// {
//     // Assertions are only allowed in state scripts, not state machines.
//     if (kParse.assertion != nullptr)
//     {
//         // Error message will point to the first token in the assertion
//         // expression, or the leftmost leaf in the tree.
//         Ref<const ExpressionParser::Parse> node = kParse.assertion;
//         while (node->left != nullptr)
//         {
//             node = node->left;
//         }

//         ConfigUtil::setError(kErr, node->data, errText,
//                             "state machines may not contain assertions");
//         return E_SMC_ASSERT;
//     }

//     // Allocate new block.
//     StateMachine::Block* const block = new StateMachine::Block{};

//     Result res = SUCCESS;
//     if (kParse.guard != nullptr)
//     {
//         // Compile block guard.
//         Ref<const ExpressionCompiler::Assembly> guardAsm;
//         res = ExpressionCompiler::compile(kParse.guard,
//                                           {&kSv, kCompState.localSv},
//                                           ElementType::BOOL,
//                                           guardAsm,
//                                           kErr);
//         if (res != SUCCESS)
//         {
//             // Override error text set by expression compiler for consistent
//             // state machine compiler error messages.
//             if (kErr != nullptr)
//             {
//                 kErr->text = errText;
//             }

//             // Delete allocations since aborting compilation.
//             deleteBlock(block);
//             return res;
//         }

//         // Track compiled expression in compilation state.
//         kCompState.exprs.push_back(guardAsm);

//         // Assign block guard.
//         block->guard = static_cast<IExprNode<bool>*>(guardAsm->root());

//         if (kParse.ifBlock != nullptr)
//         {
//             // Compile if branch block.
//             res = compileBlock(*kParse.ifBlock,
//                                kSv,
//                                kCompState,
//                                kInExitLabel,
//                                block->ifBlock,
//                                kErr);
//             if (res != SUCCESS)
//             {
//                 // Delete allocations since aborting compilation.
//                 deleteBlock(block);
//                 return res;
//             }
//         }

//         if (kParse.elseBlock != nullptr)
//         {
//             // Compile else branch block.
//             res = compileBlock(*kParse.elseBlock,
//                                kSv,
//                                kCompState,
//                                kInExitLabel,
//                                block->elseBlock,
//                                kErr);
//             if (res != SUCCESS)
//             {
//                 // Delete allocations since aborting compilation.
//                 deleteBlock(block);
//                 return res;
//             }
//         }
//     }

//     if (kParse.action != nullptr)
//     {
//         // Compile action.
//         res = compileAction(*kParse.action,
//                             kSv,
//                             kCompState,
//                             kInExitLabel,
//                             block->action,
//                             kErr);
//         if (res != SUCCESS)
//         {
//             // Delete allocations since aborting compilation.
//             deleteBlock(block);
//             return res;
//         }
//     }

//     if (kParse.next != nullptr)
//     {
//         // Compile next block.
//         res = compileBlock(*kParse.next,
//                            kSv,
//                            kCompState,
//                            kInExitLabel,
//                            block->next,
//                            kErr);
//         if (res != SUCCESS)
//         {
//             // Delete allocations since aborting compilation.
//             deleteBlock(block);
//             return res;
//         }
//     }

//     kBlock = block;
//     return SUCCESS;
// }

// Result compileState(const StateMachineParser::StateParse& kParse,
//                     const StateVector& kSv,
//                     CompilerState& kCompState,
//                     ErrorInfo* const kErr)
// {
//     // State ID is the current number of compiled states + 1 so that state IDs
//     // begin at 1.
//     StateMachine::StateConfig state =
//     {
//         static_cast<U32>(kCompState.states.size() + 1),
//         nullptr,
//         nullptr,
//         nullptr
//     };

//     Result res = SUCCESS;
//     if (kParse.entry != nullptr)
//     {
//         // Compile entry label.
//         res = compileBlock(*kParse.entry,
//                            kSv,
//                            kCompState,
//                            false,
//                            state.entry,
//                            kErr);
//         if (res != SUCCESS)
//         {
//             return res;
//         }
//     }

//     if (kParse.step != nullptr)
//     {
//         // Compile step label.
//         res = compileBlock(*kParse.step,
//                            kSv,
//                            kCompState,
//                            false,
//                            state.step,
//                            kErr);
//         if (res != SUCCESS)
//         {
//             return res;
//         }
//     }

//     if (kParse.exit != nullptr)
//     {
//         // Compile exit label.
//         res = compileBlock(*kParse.exit,
//                            kSv,
//                            kCompState,
//                            true,
//                            state.exit,
//                            kErr);
//         if (res != SUCCESS)
//         {
//             return res;
//         }
//     }

//     // Track state in compilation state.
//     kCompState.states.push_back(state);

//     return SUCCESS;
// }

// } // Anonymous namespace

// /////////////////////////////////// Public /////////////////////////////////////

// StateMachineCompiler::Assembly::Assembly(
//     const StateMachine::Config kConfig,
//     const StateMachineParser::Parse& kParse,
//     const Ref<const StateVectorCompiler::Assembly> kLocalSvAsm,
//     const Vec<Ref<const ExpressionCompiler::Assembly>> kExprs,
//     StateVector* const kLocalSv) :
//     mConfig(kConfig),
//     mParse(kParse),
//     mLocalSvAsm(kLocalSvAsm),
//     mExprs(kExprs),
//     mLocalSv(kLocalSv)
// {
// }

// StateMachineCompiler::Assembly::~Assembly()
// {
//     for (StateMachine::StateConfig* state = mConfig.states;
//          state->id != StateMachine::NO_STATE;
//          ++state)
//     {
//         // Delete state block structures.
//         deleteBlock(state->entry);
//         deleteBlock(state->step);
//         deleteBlock(state->exit);
//     }

//     // Delete the state config array.
//     delete[] mConfig.states;

//     // Delete expression stats array.
//     delete[] mConfig.stats;

//     // Delete local state vector.
//     delete mLocalSv;

//     // Note: the state vector elements are not deleted since elements in the
//     // state machine are owned by state vector assemblies.
// }

// const StateMachine::Config& StateMachineCompiler::Assembly::config() const
// {
//     return mConfig;
// }

// const StateMachineParser::Parse& StateMachineCompiler::Assembly::parse() const
// {
//     return mParse;
// }

// StateVector& StateMachineCompiler::Assembly::localStateVector() const
// {
//     return *mLocalSv;
// }

// Result StateMachineCompiler::compile(
//     const String kFilePath,
//     const StateVector& kSv,
//     Ref<const StateMachineCompiler::Assembly>& kAsm,
//     ErrorInfo* const kErr)
// {
//     if (kErr != nullptr)
//     {
//         kErr->filePath = kFilePath;
//     }

//     std::ifstream ifs(kFilePath);
//     if (!ifs.is_open())
//     {
//         if (kErr != nullptr)
//         {
//             kErr->text = "error";
//             kErr->subtext = "failed to open file `" + kFilePath + "`";
//         }
//         return E_SMC_FILE;
//     }

//     return compile(ifs, kSv, kAsm, kErr);
// }

// Result StateMachineCompiler::compile(
//     std::istream& kIs,
//     const StateVector& kSv,
//     Ref<const StateMachineCompiler::Assembly>& kAsm,
//     ErrorInfo* const kErr)
// {
//     Vec<Token> toks;
//     Result res = Tokenizer::tokenize(kIs, toks, kErr);
//     if (res != SUCCESS)
//     {
//         if (kErr != nullptr)
//         {
//             kErr->text = errText;
//         }
//         return res;
//     }

//     StateMachineParser::Parse parse = {};
//     res = StateMachineParser::parse(toks, parse, kErr);
//     if (res != SUCCESS)
//     {
//         if (kErr != nullptr)
//         {
//             kErr->text = errText;
//         }
//         return res;
//     }

//     return compile(parse, kSv, kAsm, kErr);
// }

// Result StateMachineCompiler::compile(
//     const StateMachineParser::Parse& kParse,
//     const StateVector& kSv,
//     Ref<const StateMachineCompiler::Assembly>& kAsm,
//     ErrorInfo* const kErr)
// {
//     CompilerState compState = {};

//     // Validate the state machine state vector. This will partially populate
//     // the element symbol table in the compiler state.
//     Result res = checkStateVector(kParse, kSv, compState, kErr);
//     if (res != SUCCESS)
//     {
//         return res;
//     }

//     // Compile the local state vector. This will complete the element symbol
//     // table in the compiler state.
//     res = compileLocalStateVector(kParse, compState, kErr);
//     if (res != SUCCESS)
//     {
//         return res;
//     }

//     // Set local element initial values.
//     res = initLocalElementValues(kParse, compState, kErr);
//     if (res != SUCCESS)
//     {
//         return res;
//     }

//     // Build map of state names to IDs.
//     for (U32 i = 0; i < kParse.states.size(); ++i)
//     {
//         const String& tokNameStr = kParse.states[i].tokName.str;
//         const String stateName =
//             tokNameStr.substr(1, (tokNameStr.size() - 2));
//         compState.stateIds[stateName] = (i + 1);
//     }

//     // Compile each state machine state.
//     for (const StateMachineParser::StateParse& state : kParse.states)
//     {
//         res = compileState(state, kSv, compState, kErr);
//         if (res != SUCCESS)
//         {
//             // Delete allocations since aborting compilation.
//             delete compState.localSv;
//             for (const StateMachine::StateConfig& state : compState.states)
//             {
//                 deleteBlock(state.entry);
//                 deleteBlock(state.step);
//                 deleteBlock(state.exit);
//             }

//             return res;
//         }
//     }

//     // Allocate array for state configs and copy the compiled state configs into
//     // this array.
//     StateMachine::StateConfig* states =
//         new StateMachine::StateConfig[compState.states.size() + 1];
//     std::memcpy(states,
//                 &compState.states[0],
//                 (sizeof(StateMachine::StateConfig) * compState.states.size()));
//     states[compState.states.size()] =
//         {StateMachine::NO_STATE, nullptr, nullptr, nullptr}; // Null terminator

//     // Collect expression stats needed by all expressions into a vector.
//     Vec<IExpressionStats*> exprStatsVec;
//     for (const Ref<const ExpressionCompiler::Assembly>& expr : compState.exprs)
//     {
//         exprStatsVec.insert(exprStatsVec.end(),
//                             expr->stats().begin(),
//                             expr->stats().end());
//     }

//     // Allocate array for expression stats pointers and copy the compiled
//     // expression stats pointers into this array.
//     IExpressionStats** const exprStats =
//         new IExpressionStats*[exprStatsVec.size() + 1];
//     std::memcpy(exprStats,
//                 &exprStatsVec[0],
//                 (sizeof(IExpressionStats*) * exprStatsVec.size()));
//     exprStats[exprStatsVec.size()] = nullptr; // Null terminator

//     // Create state machine config. These element lookups are guaranteed to
//     // succeed due to prior validation during compilation.
//     const StateMachine::Config smConfig =
//     {
//         static_cast<Element<U32>*>(compState.elems["S"]),
//         static_cast<Element<U64>*>(compState.elems["T"]),
//         static_cast<Element<U64>*>(compState.elems["G"]),
//         states,
//         exprStats
//     };

//     // Compilation successful- return new state machine assembly.
//     kAsm.reset(new Assembly(smConfig,
//                             kParse,
//                             compState.localSvAsm,
//                             compState.exprs,
//                             compState.localSv));
//     return SUCCESS;
// }
