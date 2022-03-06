#include <unordered_map>
#include <unordered_set>

#include "sfa/core/Assert.hpp"
#include "sfa/sup/ConfigUtil.hpp"
#include "sfa/sup/StateMachineCompiler.hpp"
#include "sfa/sup/StateVectorCompiler.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace StateMachineCompiler
{
    const char* const errText = "state machine config error";

    const std::unordered_set<std::string> reservedElemNames = {"T", "G", "S"};

    struct CompilerState final
    {
        std::unordered_map<std::string, IElement*> elems;
        std::shared_ptr<StateVectorCompiler::Assembly> localSvAsm;
        StateVector localSv;
    };

    Result checkStateVector(const StateMachineParser::Parse& kParse,
                            const StateVector& kSv,
                            CompilerState& kCompState,
                            ConfigErrorInfo* const kConfigErr);

    Result compileLocalStateVector(const StateMachineParser::Parse& kParse,
                                   CompilerState& kCompState,
                                   ConfigErrorInfo* const kConfigErr);

    Result initLocalElementValues(const StateMachineParser::Parse& kParse,
                                  CompilerState& kCompState,
                                  ConfigErrorInfo* const kConfigErr);
}

Result StateMachineCompiler::checkStateVector(
    const StateMachineParser::Parse& kParse,
    const StateVector& kSv,
    CompilerState& kCompState,
    ConfigErrorInfo* const kConfigErr)
{
    (void) kConfigErr; // rm later

    for (const StateMachineParser::StateVectorElementParse& elem :
         kParse.svElems)
    {
        // Check that element does not reuse a built-in element name.
        if (reservedElemNames.find(elem.tokName.str) != reservedElemNames.end())
        {
            SFA_ASSERT(false);
        }

        // Get element object from state vector.
        IElement* elemObj = nullptr;
        if (kSv.getIElement(elem.tokName.str.c_str(), elemObj) != SUCCESS)
        {
            // Element does not exist in state vector.
            SFA_ASSERT(false);
        }

        // Look up element type as configured in the state machine.
        auto smTypeInfoIt = ElementTypeInfo::fromName.find(elem.tokType.str);
        if (smTypeInfoIt == ElementTypeInfo::fromName.end())
        {
            // Unknown type.
            SFA_ASSERT(false);
        }
        const ElementTypeInfo& smTypeInfo = (*smTypeInfoIt).second;

        // Look up element type info as configured in the actual state vector.
        auto typeInfoIt = ElementTypeInfo::fromEnum.find(elemObj->type());
        // Assert that lookup succeeds since valid element types are guaranteed
        // by the state vector.
        SFA_ASSERT(typeInfoIt != ElementTypeInfo::fromEnum.end());
        const ElementTypeInfo& typeInfo = (*typeInfoIt).second;

        // Check that element has the same type in the state vector and state
        // machine.
        if (typeInfo.enumVal != smTypeInfo.enumVal)
        {
            SFA_ASSERT(false);
        }

        // Check that element does not appear twice in the state machine.
        if (kCompState.elems.find(elem.tokName.str) != kCompState.elems.end())
        {
            SFA_ASSERT(false);
        }

        // Add element to the symbol table.
        kCompState.elems[elem.tokName.str] = elemObj;
    }

    return SUCCESS;
}

Result StateMachineCompiler::compileLocalStateVector(
    const StateMachineParser::Parse& kParse,
    CompilerState& kCompState,
    ConfigErrorInfo* const kConfigErr)
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
    localSvParse.regions[0].elems.push_back(
        {
            {Token::IDENTIFIER, "U64", -1, -1},
            {Token::IDENTIFIER, "G", -1, -1}
        });
    localSvParse.regions[0].elems.push_back(
        {
            {Token::IDENTIFIER, "U32", -1, -1},
            {Token::IDENTIFIER, "S", -1, -1}
        });

    for (U32 i = 0; i < kParse.localElems.size(); ++i)
    {
        const StateMachineParser::LocalElementParse& elem =
            kParse.localElems[i];

        // Check that element does not reuse a built-in element name.
        if (reservedElemNames.find(elem.tokName.str) != reservedElemNames.end())
        {
            SFA_ASSERT(false);
        }

        // Check for name uniqueness against state vector elements. Uniqueness
        // against local elements will be checked by the state vector compiler
        // later on.
        for (U32 j = 0; j < kParse.svElems.size(); ++j)
        {
            if (elem.tokName.str == kParse.svElems[j].tokName.str)
            {
                SFA_ASSERT(false);
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
                                              kConfigErr);
    if (res != SUCCESS)
    {
        // Overwrite error text set by state vector compiler for consistent
        // error messages from the state machine compiler.
        if (kConfigErr != nullptr)
        {
            kConfigErr->text = errText;
        }
        return res;
    }

    // Configure the local state vector from the compiled config. Assert that
    // this succeeds since the config is known to be valid at this point.
    res = StateVector::create(kCompState.localSvAsm->getConfig(),
                              kCompState.localSv);
    SFA_ASSERT(res == SUCCESS);

    // Look up each element object in the local state vector and add it to the
    // element symbol table.
    for (const StateMachineParser::LocalElementParse& elem : kParse.localElems)
    {
        IElement* elemObj = nullptr;
        res = kCompState.localSv.getIElement(elem.tokName.str.c_str(), elemObj);
        // Assert that element lookup succeeds since we configured the local
        // state vector in this function and know the element exists.
        SFA_ASSERT(res == SUCCESS);
        kCompState.elems[elem.tokName.str] = elemObj;
    }

    return SUCCESS;
}

Result StateMachineCompiler::initLocalElementValues(
    const StateMachineParser::Parse& kParse,
    CompilerState& kCompState,
    ConfigErrorInfo* const kConfigErr)
{
    (void) kParse;
    (void) kCompState;
    (void) kConfigErr; // rm later

    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

StateMachineCompiler::Assembly::Assembly(
    const StateMachine::Config kConfig,
    const StateMachineParser::Parse& kParse) : mConfig(kConfig), mParse(kParse)
{
}

StateMachineCompiler::Assembly::~Assembly()
{
    // todo
}

Result StateMachineCompiler::compile(const StateMachineParser::Parse& kParse,
                                     const StateVector& kSv,
                                     std::shared_ptr<Assembly>& kAsm,
                                     ConfigErrorInfo* kConfigErr)
{
    StateMachine::Config smConfig = {};
    CompilerState compState = {};

    // Validate the state machine state vector. This will partially populate
    // the element symbol table in the compiler state.
    Result res = checkStateVector(kParse, kSv, compState, kConfigErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile the local state vector. This will complete the element symbol
    // table in the compiler state.
    res = compileLocalStateVector(kParse, compState, kConfigErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set local element initial values.
    res = initLocalElementValues(kParse, compState, kConfigErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile each state machine state.
    // TODO

    // Compilation successful- return new state machine assembly.
    kAsm.reset(new Assembly(smConfig, kParse));
    return SUCCESS;
}

Result StateMachineCompiler::compile(std::istream& kIs,
                                     const StateVector& kSv,
                                     std::shared_ptr<Assembly>& kAsm,
                                     ConfigErrorInfo* const kConfigErr)
{
    std::vector<Token> toks;
    Result res = ConfigTokenizer::tokenize(kIs, toks, kConfigErr);
    if (res != SUCCESS)
    {
        return res;
    }

    StateMachineParser::Parse parse = {};
    res = StateMachineParser::parse(toks, parse, kConfigErr);
    if (res != SUCCESS)
    {
        return res;
    }

    return compile(parse, kSv, kAsm, kConfigErr);
}
