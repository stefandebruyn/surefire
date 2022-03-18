#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "state machine config error";

const std::unordered_set<std::string> reservedElemNames = {"T"};

struct CompilerState final
{
    std::unordered_map<std::string, IElement*> elems;
    std::unordered_map<std::string, U32> stateIds;
    std::shared_ptr<StateVectorCompiler::Assembly> localSvAsm;
    StateVector* localSv;
    std::vector<StateMachine::StateConfig> states;
    std::vector<std::shared_ptr<ExpressionCompiler::Assembly>> exprs;
};

Result checkStateVector(const StateMachineParser::Parse& kParse,
                        const StateVector& kSv,
                        CompilerState& kCompState,
                        ErrorInfo* const kErr);

Result compileLocalStateVector(const StateMachineParser::Parse& kParse,
                               CompilerState& kCompState,
                               ErrorInfo* const kErr);

Result initLocalElementValues(const StateMachineParser::Parse& kParse,
                              CompilerState& kCompState,
                              ErrorInfo* const kErr);

Result compileAction(const StateMachineParser::ActionParse& kParse,
                     const StateVector& kSv,
                     CompilerState& kCompState,
                     IAction*& kAction,
                     ErrorInfo* const kErr);

Result compileBlock(const StateMachineParser::BlockParse& kParse,
                    const StateVector& kSv,
                    CompilerState& kCompState,
                    StateMachine::Block*& kBlock,
                    ErrorInfo* const kErr);

Result compileState(const StateMachineParser::StateParse& kParse,
                    const StateVector& kSv,
                    CompilerState& kCompState,
                    ErrorInfo* const kErr);

Result checkStateVector(const StateMachineParser::Parse& kParse,
                        const StateVector& kSv,
                        CompilerState& kCompState,
                        ErrorInfo* const kErr)
{
    (void) kErr; // rm later

    bool foundGElem = false;
    bool foundSElem = false;
    for (const StateMachineParser::StateVectorElementParse& elem :
         kParse.svElems)
    {
        // Check that element does not reuse a built-in element name.
        if (reservedElemNames.find(elem.tokName.str) != reservedElemNames.end())
        {
            SF_ASSERT(false);
        }

        // Get element object from state vector.
        IElement* elemObj = nullptr;
        if (kSv.getIElement(elem.tokName.str.c_str(), elemObj) != SUCCESS)
        {
            // Element does not exist in state vector.
            SF_ASSERT(false);
        }

        // Look up element type as configured in the state machine.
        auto smTypeInfoIt = ElementTypeInfo::fromName.find(elem.tokType.str);
        if (smTypeInfoIt == ElementTypeInfo::fromName.end())
        {
            // Unknown type.
            SF_ASSERT(false);
        }
        const ElementTypeInfo& smTypeInfo = (*smTypeInfoIt).second;

        // Look up element type info as configured in the actual state vector.
        auto typeInfoIt = ElementTypeInfo::fromEnum.find(elemObj->type());
        // Assert that lookup succeeds since valid element types are guaranteed
        // by the state vector.
        SF_ASSERT(typeInfoIt != ElementTypeInfo::fromEnum.end());
        const ElementTypeInfo& typeInfo = (*typeInfoIt).second;

        // Check that element has the same type in the state vector and state
        // machine.
        if (typeInfo.enumVal != smTypeInfo.enumVal)
        {
            SF_ASSERT(false);
        }

        // Check that element does not appear twice in the state machine.
        if (kCompState.elems.find(elem.tokName.str) != kCompState.elems.end())
        {
            SF_ASSERT(false);
        }

        // Add element to the symbol table.
        SF_ASSERT(elemObj != nullptr);
        kCompState.elems[elem.tokName.str] = elemObj;

        // If the element is aliased, add the alias to the symbol table too.
        if (elem.alias.size() > 0)
        {
            // `G` alias designates the global time element updated
            // externally to the state machine.
            if (elem.alias == "G")
            {
                if (smTypeInfo.enumVal != ElementType::UINT64)
                {
                    // Global time element is not U64.
                    SF_ASSERT(false);
                }
                foundGElem = true;
            }

            // `S` alias designates the state element used to expose the state
            // machine state to external code.
            if (elem.alias == "S")
            {
                if (smTypeInfo.enumVal != ElementType::UINT32)
                {
                    // State element is not U32.
                    SF_ASSERT(false);
                }
                foundSElem = true;
            }

            SF_ASSERT(elemObj != nullptr);
            kCompState.elems[elem.alias] = elemObj;
        }
    }

    if (!foundGElem)
    {
        // No global time element provided.
        SF_ASSERT(false);
    }

    if (!foundSElem)
    {
        // No state element provided.
        SF_ASSERT(false);
    }

    return SUCCESS;
}

Result compileLocalStateVector(const StateMachineParser::Parse& kParse,
                               CompilerState& kCompState,
                               ErrorInfo* const kErr)
{
    // To compile the local state vector, we'll build a state vector parse using
    // tokens from the state machine parse and then compile it using the state
    // vector compiler. The local state vector has all elements in a single
    // region named "LOCAL".
    StateVectorParser::Parse localSvParse =
    {
        // Regions
        {
            // Local region
            {
                {Token::SECTION, "[LOCAL]", -1, -1},
                "LOCAL",
                {}
            }
        }
    };

    // Add built-in state machine elements.
    localSvParse.regions[0].elems.push_back(
        {
            {Token::IDENTIFIER, "U64", -1, -1},
            {Token::IDENTIFIER, "T", -1, -1}
        });

    for (U32 i = 0; i < kParse.localElems.size(); ++i)
    {
        const StateMachineParser::LocalElementParse& elem =
            kParse.localElems[i];

        // Check that element does not reuse a built-in element name.
        if (reservedElemNames.find(elem.tokName.str) != reservedElemNames.end())
        {
            SF_ASSERT(false);
        }

        // Check for name uniqueness against state vector elements and aliases.
        // Uniqueness against local elements will be checked by the state vector
        // compiler later on.
        for (const StateMachineParser::StateVectorElementParse& svElem :
             kParse.svElems)
        {
            if ((elem.tokName.str == svElem.tokName.str)
                || ((svElem.alias.size() > 0)
                    && (elem.tokName.str == svElem.alias)))
            {
                SF_ASSERT(false);
            }
        }

        // Add element to local state vector parse.
        localSvParse.regions[0].elems.push_back({elem.tokType, elem.tokName});
    }

    // Compile the local state vector. Since the local state vector parse is at
    // least syntatically correct, there are very few potential errors that the
    // state vector compiler can generate here.
    Result res = StateVectorCompiler::compile(localSvParse,
                                              kCompState.localSvAsm,
                                              kErr);
    if (res != SUCCESS)
    {
        // Overwrite error text set by state vector compiler for consistent
        // error messages from the state machine compiler.
        if (kErr != nullptr)
        {
            kErr->text = errText;
        }
        return res;
    }

    // Configure the local state vector from the compiled config. Assert that
    // this succeeds since the config is known to be valid at this point.
    kCompState.localSv = new StateVector();
    res = StateVector::create(kCompState.localSvAsm->getConfig(),
                              *kCompState.localSv);
    SF_ASSERT(res == SUCCESS);

    // Look up each element object in the local state vector and add it to the
    // element symbol table.
    for (const StateVectorParser::ElementParse& elem :
         localSvParse.regions[0].elems)
    {
        IElement* elemObj = nullptr;
        res = kCompState.localSv->getIElement(elem.tokName.str.c_str(),
                                              elemObj);
        // Assert that element lookup succeeds since we configured the local
        // state vector in this function and know the element exists.
        SF_ASSERT(res == SUCCESS);
        SF_ASSERT(elemObj != nullptr);
        kCompState.elems[elem.tokName.str] = elemObj;
    }

    return SUCCESS;
}

Result initLocalElementValues(const StateMachineParser::Parse& kParse,
                              CompilerState& kCompState,
                              ErrorInfo* const kErr)
{
    (void) kParse;
    (void) kCompState;
    (void) kErr; // rm later

    return SUCCESS;
}

Result compileAction(const StateMachineParser::ActionParse& kParse,
                     const StateVector& kSv,
                     CompilerState& kCompState,
                     IAction*& kAction,
                     ErrorInfo* const kErr)
{
    Result res = SUCCESS;
    if (kParse.lhs != nullptr)
    {
        // Compile assignment action.

        // Look up RHS element.
        auto elemIt = kCompState.elems.find(kParse.tokRhs.str);
        if (elemIt == kCompState.elems.end())
        {
            // Unknown element.
            SF_ASSERT(false);
        }
        IElement* const elemObj = (*elemIt).second;

        // Compile LHS expression.
        std::shared_ptr<ExpressionCompiler::Assembly> lhsAsm;
        res = ExpressionCompiler::compile(kParse.lhs,
                                          {&kSv, kCompState.localSv},
                                          elemObj->type(),
                                          lhsAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            // Override error text set by expression compiler for consistent
            // state machine compiler error messages.
            if (kErr != nullptr)
            {
                kErr->text = errText;
            }

            return res;
        }

        // Track compiled expression in compilation state.
        kCompState.exprs.push_back(lhsAsm);

        // Create assignment action based on element type. The element object
        // and LHS root nodes are narrowed to template instantiations that match
        // the element type. These casts are guaranteed correct in this context
        // by the element and expression compiler implementations.
        switch (elemObj->type())
        {
            case ElementType::INT8:
                kAction = new AssignmentAction<I8>(
                    *static_cast<Element<I8>*>(elemObj),
                    *static_cast<const IExprNode<I8>*>(lhsAsm->root()));
                break;

            case ElementType::INT16:
                kAction = new AssignmentAction<I16>(
                    *static_cast<Element<I16>*>(elemObj),
                    *static_cast<const IExprNode<I16>*>(lhsAsm->root()));
                break;

            case ElementType::INT32:
                kAction = new AssignmentAction<I32>(
                    *static_cast<Element<I32>*>(elemObj),
                    *static_cast<const IExprNode<I32>*>(lhsAsm->root()));
                break;

            case ElementType::INT64:
                kAction = new AssignmentAction<I64>(
                    *static_cast<Element<I64>*>(elemObj),
                    *static_cast<const IExprNode<I64>*>(lhsAsm->root()));
                break;

            case ElementType::UINT8:
                kAction = new AssignmentAction<U8>(
                    *static_cast<Element<U8>*>(elemObj),
                    *static_cast<const IExprNode<U8>*>(lhsAsm->root()));
                break;

            case ElementType::UINT16:
                kAction = new AssignmentAction<U16>(
                    *static_cast<Element<U16>*>(elemObj),
                    *static_cast<const IExprNode<U16>*>(lhsAsm->root()));
                break;

            case ElementType::UINT32:
                kAction = new AssignmentAction<U32>(
                    *static_cast<Element<U32>*>(elemObj),
                    *static_cast<const IExprNode<U32>*>(lhsAsm->root()));
                break;

            case ElementType::UINT64:
                kAction = new AssignmentAction<U64>(
                    *static_cast<Element<U64>*>(elemObj),
                    *static_cast<const IExprNode<U64>*>(lhsAsm->root()));
                break;

            case ElementType::FLOAT32:
                kAction = new AssignmentAction<F32>(
                    *static_cast<Element<F32>*>(elemObj),
                    *static_cast<const IExprNode<F32>*>(lhsAsm->root()));
                break;

            case ElementType::FLOAT64:
                kAction = new AssignmentAction<F64>(
                    *static_cast<Element<F64>*>(elemObj),
                    *static_cast<const IExprNode<F64>*>(lhsAsm->root()));
                break;

            case ElementType::BOOL:
                kAction = new AssignmentAction<bool>(
                    *static_cast<Element<bool>*>(elemObj),
                    *static_cast<const IExprNode<bool>*>(lhsAsm->root()));
                break;

            default:
                SF_ASSERT(false);
        }
    }
    else
    {
        // Compile transition action.
        auto stateIdIt = kCompState.stateIds.find(kParse.tokDestState.str);
        if (stateIdIt == kCompState.stateIds.end())
        {
            // Unknown destination state.
            SF_ASSERT(false);
        }
        const U32 destState = (*stateIdIt).second;
        kAction = new TransitionAction(destState);
    }

    return SUCCESS;
}

Result compileBlock(const StateMachineParser::BlockParse& kParse,
                    const StateVector& kSv,
                    CompilerState& kCompState,
                    StateMachine::Block*& kBlock,
                    ErrorInfo* const kErr)
{
    // Allocate new block.
    kBlock = new StateMachine::Block{};

    Result res = SUCCESS;
    if (kParse.guard != nullptr)
    {
        // Compile block guard.
        std::shared_ptr<ExpressionCompiler::Assembly> guardAsm;
        res = ExpressionCompiler::compile(kParse.guard,
                                          {&kSv, kCompState.localSv},
                                          ElementType::BOOL,
                                          guardAsm,
                                          kErr);
        if (res != SUCCESS)
        {
            // Override error text set by expression compiler for consistent
            // state machine compiler error messages.
            if (kErr != nullptr)
            {
                kErr->text = errText;
            }

            // Delete allocations since aborting compilation.
            delete kBlock;
            return res;
        }

        // Track compiled expression in compilation state.
        kCompState.exprs.push_back(guardAsm);

        // Assign block guard.
        kBlock->guard = static_cast<const IExprNode<bool>*>(guardAsm->root());

        if (kParse.ifBlock != nullptr)
        {
            // Compile if branch block.
            res = compileBlock(*kParse.ifBlock,
                               kSv,
                               kCompState,
                               kBlock->ifBlock,
                               kErr);
            if (res != SUCCESS)
            {
                // Delete allocations since aborting compilation.
                delete kBlock;
                return res;
            }
        }

        if (kParse.elseBlock != nullptr)
        {
            // Compile else branch block.
            res = compileBlock(*kParse.elseBlock,
                               kSv,
                               kCompState,
                               kBlock->elseBlock,
                               kErr);
            if (res != SUCCESS)
            {
                // Delete allocations since aborting compilation.
                delete kBlock;
                return res;
            }
        }
    }
    else if (kParse.action != nullptr)
    {
        // Compile action.
        res = compileAction(*kParse.action,
                            kSv,
                            kCompState,
                            kBlock->action,
                            kErr);
        if (res != SUCCESS)
        {
            // Delete allocations since aborting compilation.
            delete kBlock;
            return res;
        }
    }

    if (kParse.next != nullptr)
    {
        // Compile next block.
        res = compileBlock(*kParse.next, kSv, kCompState, kBlock->next, kErr);
        if (res != SUCCESS)
        {
            // Delete allocations since aborting compilation.
            delete kBlock;
            return res;
        }
    }

    return SUCCESS;
}

Result compileState(const StateMachineParser::StateParse& kParse,
                    const StateVector& kSv,
                    CompilerState& kCompState,
                    ErrorInfo* const kErr)
{
    // State ID is the current number of compiled states + 1 so that state IDs
    // begin at 1.
    StateMachine::StateConfig state =
    {
        static_cast<U32>(kCompState.states.size() + 1),
        nullptr,
        nullptr,
        nullptr
    };

    Result res = SUCCESS;
    if (kParse.entry != nullptr)
    {
        // Compile entry label.
        res = compileBlock(*kParse.entry, kSv, kCompState, state.entry, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    if (kParse.step != nullptr)
    {
        // Compile step label.
        res = compileBlock(*kParse.step, kSv, kCompState, state.step, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    if (kParse.exit != nullptr)
    {
        // Compile exit label.
        res = compileBlock(*kParse.exit, kSv, kCompState, state.exit, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Track state in compilation state.
    kCompState.states.push_back(state);

    return SUCCESS;
}

} // Anonymous namespace

/////////////////////////////////// Public /////////////////////////////////////

StateMachineCompiler::Assembly::Assembly(
    const StateMachine::Config kConfig,
    const StateMachineParser::Parse& kParse,
    const std::shared_ptr<StateVectorCompiler::Assembly> kLocalSvAsm,
    const std::vector<std::shared_ptr<ExpressionCompiler::Assembly>> kExprs,
    StateVector* const kLocalSv) :
    mConfig(kConfig),
    mParse(kParse),
    mLocalSvAsm(kLocalSvAsm),
    mExprs(kExprs),
    mLocalSv(kLocalSv)
{
}

StateMachineCompiler::Assembly::~Assembly()
{
    for (StateMachine::StateConfig* state = mConfig.states;
         state->id != StateMachine::NO_STATE;
         ++state)
    {
        // Delete state block structures.
        this->deleteBlock(state->entry);
        this->deleteBlock(state->step);
        this->deleteBlock(state->exit);
    }

    // Delete the state config array.
    delete[] mConfig.states;

    // Delete local state vector.
    delete mLocalSv;

    // Note: the state vector elements are not deleted since elements in the
    // state machine are owned by state vector assemblies.
}

const StateMachine::Config& StateMachineCompiler::Assembly::config() const
{
    return mConfig;
}

const StateMachineParser::Parse& StateMachineCompiler::Assembly::parse() const
{
    return mParse;
}

StateVector& StateMachineCompiler::Assembly::localStateVector() const
{
    return *mLocalSv;
}

void StateMachineCompiler::Assembly::deleteBlock(
    const StateMachine::Block* const kBlock)
{
    // Base case: null block.
    if (kBlock == nullptr)
    {
        return;
    }

    // Delete linked blocks.
    this->deleteBlock(kBlock->ifBlock);
    this->deleteBlock(kBlock->elseBlock);
    this->deleteBlock(kBlock->next);

    // Delete block action and finally the block itself.
    delete kBlock->action;
    delete kBlock;

    // Note: the block guard is not deleted since expressions in the state
    // machine are owned by expression assemblies.
}

Result StateMachineCompiler::compile(const std::string kFilePath,
                                     const StateVector& kSv,
                                     std::shared_ptr<Assembly>& kAsm,
                                     ErrorInfo* const kErr)
{
    if (kErr != nullptr)
    {
        kErr->filePath = kFilePath;
    }

    std::ifstream ifs(kFilePath);
    if (!ifs.is_open())
    {
        if (kErr != nullptr)
        {
            kErr->text = "error";
            kErr->subtext = "failed to open file `" + kFilePath + "`";
        }
        return E_SMC_FILE;
    }

    return compile(ifs, kSv, kAsm, kErr);
}

Result StateMachineCompiler::compile(std::istream& kIs,
                                     const StateVector& kSv,
                                     std::shared_ptr<Assembly>& kAsm,
                                     ErrorInfo* const kErr)
{
    std::vector<Token> toks;
    Result res = Tokenizer::tokenize(kIs, toks, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = errText;
        }
        return res;
    }

    StateMachineParser::Parse parse = {};
    res = StateMachineParser::parse(toks, parse, kErr);
    if (res != SUCCESS)
    {
        if (kErr != nullptr)
        {
            kErr->text = errText;
        }
        return res;
    }

    return compile(parse, kSv, kAsm, kErr);
}

Result StateMachineCompiler::compile(const StateMachineParser::Parse& kParse,
                                     const StateVector& kSv,
                                     std::shared_ptr<Assembly>& kAsm,
                                     ErrorInfo* const kErr)
{
    CompilerState compState = {};

    // Validate the state machine state vector. This will partially populate
    // the element symbol table in the compiler state.
    Result res = checkStateVector(kParse, kSv, compState, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile the local state vector. This will complete the element symbol
    // table in the compiler state.
    res = compileLocalStateVector(kParse, compState, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set local element initial values.
    res = initLocalElementValues(kParse, compState, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Build map of state names to IDs.
    for (U32 i = 0; i < kParse.states.size(); ++i)
    {
        const std::string& tokNameStr = kParse.states[i].tokName.str;
        const std::string stateName =
            tokNameStr.substr(1, (tokNameStr.size() - 2));
        compState.stateIds[stateName] = (i + 1);
    }

    // Compile each state machine state.
    for (const StateMachineParser::StateParse& state : kParse.states)
    {
        res = compileState(state, kSv, compState, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Allocate array for state configs and copy the compiled state configs into
    // this array.
    StateMachine::StateConfig* states =
        new StateMachine::StateConfig[compState.states.size() + 1];
    std::memcpy(states,
                &compState.states[0],
                (sizeof(StateMachine::StateConfig) * compState.states.size()));
    states[compState.states.size()] =
        {StateMachine::NO_STATE, nullptr, nullptr, nullptr}; // Null terminator

    // Create state machine config. These element lookups are guaranteed to
    // succeed due to prior validation during compilation.
    const StateMachine::Config smConfig =
    {
        static_cast<Element<U32>*>(compState.elems["S"]),
        static_cast<Element<U64>*>(compState.elems["T"]),
        static_cast<Element<U64>*>(compState.elems["G"]),
        states
    };

    // Compilation successful- return new state machine assembly.
    kAsm.reset(new Assembly(smConfig,
                            kParse,
                            compState.localSvAsm,
                            compState.exprs,
                            compState.localSv));
    return SUCCESS;
}
