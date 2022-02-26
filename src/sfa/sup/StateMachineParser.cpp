#include <regex>

#include "sfa/core/Assert.hpp"
#include "sfa/sup/StateMachineParser.hpp"

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
}

Result StateMachineParser::parseImpl(const std::vector<Token>& kToks,
                                     StateVector& kSv,
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
                    res = parseLocalSection(it, kSv, parse, kConfigErr);
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

Result StateMachineParser::parseLocalSection(TokenIterator& kIt,
                                             StateVector& kSv,
                                             Parse& kParse,
                                             ConfigErrorInfo* kConfigErr)
{
    (void) kSv; // rm later
    (void) kConfigErr;

    SFA_ASSERT((kIt.type() == Token::SECTION) && (kIt.str() == "[LOCAL]"));
    kIt.take();

    while (kIt.eof() == false)
    {
        kIt.eat();
        LocalElementParse elemParse;
        elemParse.readOnly = false;

        // Take element type.
        if (kIt.type() != Token::IDENTIFIER)
        {
            SFA_ASSERT(false);
        }
        // verify type is valid
        elemParse.tokType = kIt.take();

        // Take element name.
        if (kIt.type() != Token::IDENTIFIER)
        {
            SFA_ASSERT(false);
        }
        // verify name is not reserved
        // verify name is not dupe
        elemParse.tokName = kIt.take();

        // Take assignment operator.
        if (kIt.type() != Token::OPERATOR)
        {
            // expected operator
            SFA_ASSERT(false);
        }
        else if (kIt.str() != "=")
        {
            // wrong operator
            SFA_ASSERT(false);
        }
        kIt.take();

        // Take initial value.
        if (kIt.type() != Token::CONSTANT)
        {
            // expected constant
            SFA_ASSERT(false);
        }
        // verify constant type is valid for elem type
        elemParse.tokInitVal = kIt.take();

        // Take annotations.
        while ((kIt.eof() == false) && (kIt.type() == Token::ANNOTATION))
        {
            std::smatch match;
            if (kIt.str() == "@READ_ONLY")
            {
                // Read-only annotation.
                elemParse.readOnly = true;
                kIt.take();
            }
            else if (std::regex_match(kIt.str(), match, aliasRegex) == true)
            {
                elemParse.alias = match[1].str();
                // check that alias is not reserved or dupe
                kIt.take();
            }
            else
            {
                // unknown annotation
                SFA_ASSERT(false);
            }
        }

        // Add element to parse.
        kParse.localElems.push_back(elemParse);
    }

    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

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
