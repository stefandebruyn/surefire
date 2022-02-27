#include <regex>
#include <iostream> // rm later

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
                     StateVector& kSv,
                     std::shared_ptr<Config>& kConfig,
                     ConfigErrorInfo* kConfigErr);

    bool isNameUnique(const std::string kName,
                      const Parse& kParse,
                      Token& kErrTok);
}

Result StateMachineParser::parseImpl(const std::vector<Token>& kToks,
                                     StateVector& kSv,
                                     std::shared_ptr<Config>& kConfig,
                                     ConfigErrorInfo* kConfigErr)
{
    (void) kSv;
    (void) kConfig; // rm later
    TokenIterator it(kToks.begin(), kToks.end());
    Parse parse;

    while (!it.eof())
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
    if (kParse.hasLocalSection)
    {
        ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                             "more than one local section");
        return E_SMP_LOC_MULT;
    }
    kParse.hasLocalSection = true;

    // Assert that iterator is currently positioned at the local section.
    SFA_ASSERT((kIt.type() == Token::SECTION) && (kIt.str() == "[LOCAL]"));
    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        LocalElementParse elemParse;

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Check that element type is valid.
        if (ConfigUtil::typeInfoFromName.find(kIt.str())
            == ConfigUtil::typeInfoFromName.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "unknown type `" + kIt.str() + "`");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokType, gErrText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Check that element name is not reserved.
        if (ConfigUtil::reserved.find(kIt.str()) != ConfigUtil::reserved.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "`" + kIt.str() + "` is a reserved name");
            return E_SMP_NAME_RSVD;
        }

        // Check that element name is unique.
        Token tokErr;
        if (!isNameUnique(kIt.str(), kParse, tokErr))
        {
            std::stringstream ss;
            ss << "reuse of name `" << kIt.str()
               << "` (previously used on line " << tokErr.lineNum << ")";
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText, ss.str());
            return E_SMP_NAME_DUPE;
        }

        // Take element name.
        elemParse.tokName = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokName, gErrText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token is an assignment operator.
        if ((kIt.type() != Token::OPERATOR) || (kIt.str() != "="))
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "expected `=` after element name");
            return E_SMP_LOC_OP;
        }

        // Take assignment operator.
        const Token& tokAsgOp = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, tokAsgOp, gErrText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be the element initial value,
        // is a constant.
        if (kIt.type() != Token::CONSTANT)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "expected constant element initial value");
            return E_SMP_LOC_VAL;
        }

        // Take element initial value.
        elemParse.tokInitVal = kIt.take();

        // Take annotations.
        while (kIt.type() == Token::ANNOTATION)
        {
            if (kIt.str() == "@READ_ONLY")
            {
                // Read-only annotation.

                // Check that element is not already marked read-only.
                if (elemParse.readOnly)
                {
                    ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                         "redundant read-only annotation");
                    return E_SMP_RO_MULT;
                }

                elemParse.readOnly = true;
                kIt.take();
            }
            else
            {
                // Unknown annotation.
                ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                     "unknown annotation");
                return E_SMP_ANNOT;
            }
        }

        // Add element to parse.
        kParse.localElems.push_back(elemParse);
    }

    return SUCCESS;
}

Result StateMachineParser::parseStateVectorSection(TokenIterator& kIt,
                                                   const StateVector& kSv,
                                                   Parse& kParse,
                                                   ConfigErrorInfo* kConfigErr)
{
    // Check that a local section has not already been parsed.
    if (kParse.hasStateVectorSection)
    {
        ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                             "more than one state vector section");
        return E_SMP_SV_MULT;
    }
    kParse.hasStateVectorSection = true;

    // Assert that iterator is currently positioned at the state vector section.
    SFA_ASSERT((kIt.type() == Token::SECTION)
               && (kIt.str() == "[STATE_VECTOR]"));
    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        StateVectorElementParse elemParse;

        // Check that current token, which should be the element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Check that element type is valid.
        auto typeInfoIt = ConfigUtil::typeInfoFromName.find(kIt.str());
        if (typeInfoIt == ConfigUtil::typeInfoFromName.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "unknown type `" + kIt.str() + "`");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokType, gErrText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be the element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Check that element name is not reserved.
        if (ConfigUtil::reserved.find(kIt.str()) != ConfigUtil::reserved.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "`" + kIt.str() + "` is a reserved name");
            return E_SMP_NAME_RSVD;
        }

        // Check that element name is unique.
        Token tokErr;
        if (!isNameUnique(kIt.str(), kParse, tokErr))
        {
            std::stringstream ss;
            ss << "reuse of name `" << kIt.str() << "`"
               << " (previously used on line " << tokErr.lineNum << ")";
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText, ss.str());
            return E_SMP_NAME_DUPE;
        }

        // Check that element exists in the state vector config.
        const IElement* elemObj = nullptr;
        if (kSv.getIElement(kIt.str().c_str(), elemObj) != SUCCESS)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                 "state vector does not contain an element"
                                 " named `" + kIt.str() + "`");
            return E_SMP_SV_NAME;
        }

        // Check that element has the same type in the state vector config.
        const ConfigUtil::ElementTypeInfo& elemTypeInfo = (*typeInfoIt).second;
        if (elemTypeInfo.enumVal != elemObj->type())
        {
            auto it = ConfigUtil::typeInfoFromEnum.find(elemObj->type());
            SFA_ASSERT(it != ConfigUtil::typeInfoFromEnum.end());
            std::stringstream ss;
            ss << "element `" << kIt.str() << "` is type `" << (*it).second.name
               << "` in the state vector config but `" << elemParse.tokType.str
               << "` here";
            ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText, ss.str());
            return E_SMP_SV_TYPE;
        }

        // Take element name.
        elemParse.tokName = kIt.take();

        // Take annotations.
        while (kIt.type() == Token::ANNOTATION)
        {
            std::smatch match;
            if (kIt.str() == "@READ_ONLY")
            {
                // Read-only annotation.

                // Check that element is not already marked read-only.
                if (elemParse.readOnly)
                {
                    ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                         "redundant read-only annotation");
                    return E_SMP_RO_MULT;
                }

                // Take annotation.
                elemParse.readOnly = true;
                kIt.take();
            }
            else if (std::regex_match(kIt.str(), match, aliasRegex))
            {
                // Alias annotation.

                // Check that element is not already aliased.
                if (elemParse.alias.size() > 0)
                {
                    ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                         "an element may only have one alias");
                    return E_SMP_AL_MULT;
                }

                // Check that alias is a unique name.
                Token tokErr;
                if (!isNameUnique(match[1].str(), kParse, tokErr))
                {
                    std::stringstream ss;
                    ss << "reuse of name `" << kIt.str()
                       << "` (previously used on line " << tokErr.lineNum
                       << ")";
                    ConfigUtil::setError(
                        kConfigErr, kIt.tok(), gErrText, ss.str());
                    return E_SMP_NAME_DUPE;
                }

                // Take alias.
                elemParse.tokAlias = kIt.take();
                elemParse.alias = match[1].str();
            }
            else
            {
                // Unknown annotation.
                ConfigUtil::setError(kConfigErr, kIt.tok(), gErrText,
                                     "unknown annotation");
                return E_SMP_ANNOT;
            }
        }

        // Add element to parse.
        kParse.svElems.push_back(elemParse);
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

StateMachineParser::Parse::Parse() :
    hasLocalSection(false), hasStateVectorSection(false)
{
}

Result StateMachineParser::parse(std::istream& kIs,
                                 StateVector& kSv,
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

    return parseImpl(toks, kSv, kConfig, kConfigErr);
}
