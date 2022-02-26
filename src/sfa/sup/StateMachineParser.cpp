#include <regex>

#include "sfa/core/Assert.hpp"
#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/sup/ConfigUtil.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state machine config error";

/////////////////////////////////// Private ////////////////////////////////////

namespace StateMachineParser
{
    const std::regex aliasRegex("@ALIAS=([a-zA-Z][a-zA-Z0-9_]*)");

    Result parseImpl(const std::vector<Token>& kToks,
                     std::shared_ptr<Config>& kConfig,
                     ConfigErrorInfo* kConfigErr);

    bool isNameUnique(const std::string kName,
                      const Parse& kParse,
                      Token& kErrTok);
}

Result StateMachineParser::parseImpl(const std::vector<Token>& kToks,
                                     std::shared_ptr<Config>& kConfig,
                                     ConfigErrorInfo* kConfigErr)
{
    (void) kConfig; // rm later
    TokenIterator it(kToks.begin(), kToks.end());
    Parse parse;

    while (it.eof() == false)
    {
        Result res = SUCCESS;

        switch (it.type())
        {
            case Token::NEWLINE:
                it.take();
                break;

            case Token::SECTION:
                if (it.str() == "[LOCAL]")
                {
                    res = parseLocalSection(it, parse, kConfigErr);
                }
                else
                {
                    // unknown section
                    SFA_ASSERT(false);
                }
                break;

            default:
                // invalid type
                SFA_ASSERT(false);
        }

        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

bool StateMachineParser::isNameUnique(const std::string kName,
                                      const Parse& kParse,
                                      Token& kErrTok)
{
    // Check for uniqueness against local elements.
    for (const LocalElementParse& elem : kParse.localElems)
    {
        if (elem.tokName.str == kName)
        {
            // Name is already used by a local element.
            kErrTok = elem.tokName;
            return false;
        }
    }

    // Check for uniqueness against state vector elements.
    for (const StateVectorElementParse& elem : kParse.svElems)
    {
        if (elem.tokName.str == kName)
        {
            // Name is already used by a state vector element.
            kErrTok = elem.tokName;
            return false;
        }
        else if (elem.alias == kName)
        {
            // Name is already used by a state vector element alias.
            kErrTok = elem.tokAlias;
            return false;
        }
    }

    // Check for uniqueness against state names.
    for (const StateParse& state : kParse.states)
    {
        if (state.tokName.str == kName)
        {
            // Name is already used by a state.
            kErrTok = state.tokName;
            return false;
        }
    }

    return true;
}

Result StateMachineParser::parseLocalSection(TokenIterator& kIt,
                                             Parse& kParse,
                                             ConfigErrorInfo* kConfigErr)
{

    // Check that a local section has not already been parsed.
    if (kParse.hasLocalSection == true)
    {
        if (kConfigErr != nullptr)
        {
            kConfigErr->lineNum = kIt.tok().lineNum;
            kConfigErr->colNum = kIt.tok().colNum;
            kConfigErr->text = gErrText;
            kConfigErr->subtext = "more than one local section";
        }
        return E_SMP_MULT_LOC;
    }
    kParse.hasLocalSection = true;

    // Assert that iterator is currently positioned at the local section.
    SFA_ASSERT((kIt.type() == Token::SECTION) && (kIt.str() == "[LOCAL]"));
    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while ((kIt.eof() == false) && (kIt.type() != Token::SECTION))
    {
        kIt.eat();
        LocalElementParse elemParse;

        // Take element type.
        if (kIt.type() != Token::IDENTIFIER)
        {
            // Expected an identifier.
            if (kConfigErr != nullptr)
            {
                kConfigErr->lineNum = kIt.tok().lineNum;
                kConfigErr->colNum = kIt.tok().colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext = "expected element type";
            }
            return E_SMP_LOC_TYPE;
        }
        else if (ConfigUtil::isElementType(kIt.str()) == false)
        {
            // Element type is invalid.
            if (kConfigErr != nullptr)
            {
                kConfigErr->lineNum = kIt.tok().lineNum;
                kConfigErr->colNum = kIt.tok().colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext = "unknown type `" + kIt.str() + "`";
            }
            return E_SMP_LOC_TYPE;
        }
        elemParse.tokType = kIt.take();

        // Take element name.
        Token tokErr;
        if (kIt.type() != Token::IDENTIFIER)
        {
            // Expected element name.
            if (kConfigErr != nullptr)
            {
                // Error message points to element type token since the iterator
                // may have reached end of file.
                kConfigErr->lineNum = elemParse.tokType.lineNum;
                kConfigErr->colNum = elemParse.tokType.colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext = "expected element name after type";
            }
            return E_SMP_LOC_NAME;
        }
        else if (ConfigUtil::isReserved(kIt.str()) == true)
        {
            // Element name is reserved.
            if (kConfigErr != nullptr)
            {
                kConfigErr->lineNum = kIt.tok().lineNum;
                kConfigErr->colNum = kIt.tok().colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext = "`" + kIt.str() + "` is a reserved name";
            }
            return E_SMP_LOC_RSVD;
        }
        else if (isNameUnique(kIt.str(), kParse, tokErr) == false)
        {
            // Element name is not unique.
            if (kConfigErr != nullptr)
            {
                kConfigErr->lineNum = kIt.tok().lineNum;
                kConfigErr->colNum = kIt.tok().colNum;
                kConfigErr->text = gErrText;
                std::stringstream ss;
                ss << "reuse of name `" << kIt.str() << "`"
                   << " (previously used on line " << tokErr.lineNum << ")";
                kConfigErr->subtext = ss.str();
            }
            return E_SMP_LOC_DUPE;
        }
        elemParse.tokName = kIt.take();

        // Take assignment operator.
        if (kIt.type() != Token::OPERATOR)
        {
            // Expected operator after element name.
            if (kConfigErr != nullptr)
            {
                // Error message points to element name token since the iterator
                // may have reached end of file.
                kConfigErr->lineNum = elemParse.tokName.lineNum;
                kConfigErr->colNum = elemParse.tokName.colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext = "expected `=` after element name";
            }
            return E_SMP_LOC_OP;
        }
        else if (kIt.str() != "=")
        {
            // Non-assignment operator after element name.
            if (kConfigErr != nullptr)
            {
                kConfigErr->lineNum = kIt.tok().lineNum;
                kConfigErr->colNum = kIt.tok().colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext = "expected `=` after element name";
            }
            return E_SMP_LOC_OP;
        }
        const Token& tokAssignmentOp = kIt.take();

        // Take initial value.
        if (kIt.type() != Token::CONSTANT)
        {
            // Expected constant after assignment operator.
            if (kConfigErr != nullptr)
            {
                // Error message points to assignment operator token since the
                // iterator may have reached end of file.
                kConfigErr->lineNum = tokAssignmentOp.lineNum;
                kConfigErr->colNum = tokAssignmentOp.colNum;
                kConfigErr->text = gErrText;
                kConfigErr->subtext =
                    "expected constant after assignment operator";
            }
            return E_SMP_LOC_VAL;
        }
        elemParse.tokInitVal = kIt.take();

        // Take annotations.
        while (kIt.type() == Token::ANNOTATION)
        {
            std::smatch match;
            if (kIt.str() == "@READ_ONLY")
            {
                if (elemParse.readOnly == true)
                {
                    // Element was already marked read-only.
                    if (kConfigErr != nullptr)
                    {
                        kConfigErr->lineNum = kIt.tok().lineNum;
                        kConfigErr->colNum = kIt.tok().colNum;
                        kConfigErr->text = gErrText;
                        kConfigErr->subtext = "redundant read-only annotation";
                    }
                    return E_SMP_RO_RED;
                }
                // Read-only annotation.
                elemParse.readOnly = true;
                kIt.take();
            }
            else
            {
                // Unknown annotation.
                if (kConfigErr != nullptr)
                {
                    kConfigErr->lineNum = kIt.tok().lineNum;
                    kConfigErr->colNum = kIt.tok().colNum;
                    kConfigErr->text = gErrText;
                    kConfigErr->subtext = "unknown annotation";
                }
                return E_SMP_LOC_ANN;
            }
        }

        // Add element to parse.
        kParse.localElems.push_back(elemParse);
    }

    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

StateMachineParser::StateVectorElementParse::StateVectorElementParse() :
    readOnly(false)
{
}

StateMachineParser::LocalElementParse::LocalElementParse() : readOnly(false)
{
}

StateMachineParser::Parse::Parse() : hasLocalSection(false)
{
}

Result StateMachineParser::parse(std::istream& kIs,
                                 std::shared_ptr<Config>& kConfig,
                                 ConfigErrorInfo* kConfigErr)
{
    std::vector<Token> toks;
    const Result res = ConfigTokenizer::tokenize(kIs, toks, kConfigErr);
    if (res != SUCCESS)
    {
        if (kConfigErr != nullptr)
        {
            // Overwrite error text set by tokenizer for consistent error
            // messages from the state vector parser.
            kConfigErr->text = gErrText;
        }
        return res;
    }

    return parseImpl(toks, kConfig, kConfigErr);
}
