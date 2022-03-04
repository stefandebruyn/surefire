#include <regex>
#include <iostream> // rm later

#include "sfa/core/Assert.hpp"
#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/sup/ConfigUtil.hpp"
#include "sfa/sup/ExpressionParser.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace StateMachineParser
{
    const char* const errText = "state machine config error";

    const std::regex aliasRegex("@ALIAS=([a-zA-Z][a-zA-Z0-9_]*)");

    Result parseAction(TokenIterator kIt,
                       std::shared_ptr<ActionParse>& kAction,
                       ConfigErrorInfo* kConfigErr);

    Result parseBlock(TokenIterator kIt,
                      std::shared_ptr<BlockParse>& kBlock,
                      ConfigErrorInfo* kConfigErr);

    Result parseImpl(const std::vector<Token>& kToks,
                     StateVector& kSv,
                     std::shared_ptr<Config>& kConfig,
                     ConfigErrorInfo* kConfigErr);

    bool isNameUnique(const std::string kName,
                      const Parse& kParse,
                      Token& kErrTok);
}

Result StateMachineParser::parseAction(TokenIterator kIt,
                                       std::shared_ptr<ActionParse>& kAction,
                                       ConfigErrorInfo* kConfigErr)
{
    kAction.reset(new ActionParse{});

    const Token& tok = kIt.take();
    if (tok.type == Token::IDENTIFIER)
    {
        kAction->tokRhs = tok;

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, tok, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Take assignment operator.
        const Token& tokEq = kIt.take();
        if ((tokEq.type != Token::OPERATOR) || (tokEq.str != "="))
        {
            // Expected assignment operator.
            SFA_ASSERT(false);
        }

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, tok, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Parse expression after assignment operator.
        const Result res = ExpressionParser::parse(
            kIt.slice(kIt.idx(), kIt.size()), kAction->lhs, kConfigErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }
    else if (tok.type == Token::OPERATOR)
    {
        if (tok.str != "->")
        {
            // Unexpected operator.
            SFA_ASSERT(false);
        }

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, tok, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        if (kIt.type() != Token::IDENTIFIER)
        {
            // Unexpected token after transition operator.
            SFA_ASSERT(false);
        }

        kAction->tokDestState = kIt.take();

        if (!kIt.eof())
        {
            // Unexpected token after transition operator.
            SFA_ASSERT(false);
        }
    }
    else
    {
        // Unexpected token in action.
        std::cout << tok << std::endl;
        SFA_ASSERT(false);
    }

    return SUCCESS;
}

Result StateMachineParser::parseBlock(TokenIterator kIt,
                                      std::shared_ptr<BlockParse>& kBlock,
                                      ConfigErrorInfo* kConfigErr)
{
    std::shared_ptr<BlockParse> block(new BlockParse{});
    const std::shared_ptr<BlockParse> firstBlock = block;
    Result res = SUCCESS;

    while (!kIt.eof())
    {
        // Find end index of next thing to parse. Which thing it is will depend
        // on what the end token is.
        const U32 idxEnd = kIt.next(
            {Token::COLON, Token::LBRACE, Token::NEWLINE});

        if ((idxEnd == kIt.size()) || (kIt[idxEnd].type == Token::NEWLINE))
        {
            // Parse unguarded action.
            res = parseAction(kIt.slice(kIt.idx(), idxEnd),
                              block->action,
                              kConfigErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Jump to end of action.
            kIt.seek(idxEnd);
            kIt.eat();
        }
        else
        {
            // Parse guarded action or block of actions.

            // Take optional if.
            if (kIt.str() == "IF")
            {
                kIt.take();
            }

            // Check that end of file has not been reached.
            if (ConfigUtil::checkEof(kIt, kIt[0], errText, kConfigErr))
            {
                return E_SMP_EOF;
            }

            // Parse guard.
            res = ExpressionParser::parse(kIt.slice(kIt.idx(), idxEnd),
                                          block->guard,
                                          kConfigErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Jump to first token after guard.
            kIt.seek(idxEnd);

            // Find end index of if branch.
            U32 idxBlockEnd = 0;
            if (kIt.type() == Token::LBRACE)
            {
                // Guard is followed by a left brace, so find the corresponding
                // right brace.
                U32 lvl = 0;
                idxBlockEnd = kIt.idx();
                while (idxBlockEnd < kIt.size())
                {
                    if (kIt[idxBlockEnd].type == Token::LBRACE)
                    {
                        ++lvl;
                    }
                    else if (kIt[idxBlockEnd].type == Token::RBRACE)
                    {
                        --lvl;
                        if (lvl == 0)
                        {
                            break;
                        }
                    }
                    ++idxBlockEnd;
                }

                if (lvl != 0)
                {
                    // Unbalanced braces.
                    SFA_ASSERT(false);
                }
            }
            else
            {
                // Guard is followed by a colon, so find the next newline.
                idxBlockEnd = kIt.next({Token::NEWLINE});
            }

            // Take left brace or colon following guard.
            kIt.take();

            // Parse if branch of guard.
            Result res = parseBlock(kIt.slice(kIt.idx(), idxBlockEnd),
                                    block->ifBlock,
                                    kConfigErr);
            if (res != SUCCESS)
            {
                return res;
            }

            // Jump to the first token after the guarded block.
            kIt.seek(idxBlockEnd);
            kIt.take();

            if (kIt.str() == "ELSE")
            {
                // Guard has an else branch.

                // Take else token.
                const Token& tokElse = kIt.take();

                // Check that end of file has not been reached.
                if (ConfigUtil::checkEof(kIt, tokElse, errText, kConfigErr))
                {
                    return E_SMP_EOF;
                }

                // Find end index of else branch.
                U32 idxElseEnd = 0;
                if (kIt.type() == Token::LBRACE)
                {
                    // Guard is followed by a left brace, so find the
                    // corresponding right brace.
                    U32 lvl = 0;
                    idxElseEnd = kIt.idx();
                    while (idxElseEnd < kIt.size())
                    {
                        if (kIt[idxElseEnd].type == Token::LBRACE)
                        {
                            ++lvl;
                        }
                        else if (kIt[idxElseEnd].type == Token::RBRACE)
                        {
                            --lvl;
                            if (lvl == 0)
                            {
                                break;
                            }
                        }
                        ++idxElseEnd;
                    }

                    if (lvl != 0)
                    {
                        // Unbalanced braces.
                        SFA_ASSERT(false);
                    }
                }
                else if (kIt.type() == Token::COLON)
                {
                    // Guard is followed by a colon, so find the next newline.
                    idxElseEnd = kIt.next({Token::NEWLINE});
                }
                else
                {
                    // Unexpected token after else.
                    SFA_ASSERT(false);
                }

                // Take left brace or colon following else.
                kIt.take();

                // Parse else branch.
                res = parseBlock(kIt.slice(kIt.idx(), idxElseEnd),
                                 block->elseBlock,
                                 kConfigErr);
                if (res != SUCCESS)
                {
                    return res;
                }

                // Jump to the first token after the guarded block.
                kIt.seek(idxElseEnd);
                kIt.take();
            }
        }

        if (!kIt.eof())
        {
            // Add another block in the chain.
            block->next.reset(new BlockParse{});
            block = block->next;
        }
    }

    kBlock = firstBlock;
    return SUCCESS;
}

Result StateMachineParser::parseState(TokenIterator& kIt,
                                      StateParse& kState,
                                      ConfigErrorInfo* kConfigErr)
{
    // Assert that iterator is currently positioned at a section.
    SFA_ASSERT(kIt.type() == Token::SECTION);
    // Take section token.
    kState.tokName = kIt.take();

    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        // Take label token.
        const Token& tokLab = kIt.take();
        if (tokLab.type != Token::LABEL)
        {
            // Expected a label.
            SFA_ASSERT(false);
        }

        // End index of label is the next label or section token (or EOF).
        const U32 idxLabelEnd = kIt.next({Token::LABEL, Token::SECTION});

        // Parse label block.
        std::shared_ptr<BlockParse> label;
        const Result res = parseBlock(kIt.slice(kIt.idx(), idxLabelEnd),
                                      label,
                                      kConfigErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Assign label block to state based on label name.
        if (tokLab.str == ".ENTRY")
        {
            kState.entry = label;
        }
        else if (tokLab.str == ".STEP")
        {
            kState.step = label;
        }
        else if (tokLab.str == ".EXIT")
        {
            kState.exit = label;
        }
        else
        {
            // Unknown label.
            SFA_ASSERT(false);
        }

        // Jump to end of label block.
        kIt.seek(idxLabelEnd);
    }

    return SUCCESS;
}

Result StateMachineParser::parseImpl(const std::vector<Token>& kToks,
                                     StateVector& kSv,
                                     std::shared_ptr<Config>& kConfig,
                                     ConfigErrorInfo* kConfigErr)
{
    (void) kSv;
    (void) kConfig; // rm later
    TokenIterator it(kToks.begin(), kToks.end());
    Parse parse = {};

    while (!it.eof())
    {
        Result res = SUCCESS;

        switch (it.type())
        {
            case Token::NEWLINE:
                it.take();
                break;

            case Token::SECTION:
                if (it.str() == "[STATE_VECTOR]")
                {
                    res = parseStateVectorSection(it, kSv, parse, kConfigErr);
                }
                else if (it.str() == "[LOCAL]")
                {
                    res = parseLocalSection(it, parse, kConfigErr);
                }
                else
                {
                    // State section.
                    StateParse state = {};
                    res = parseState(it, state, kConfigErr);
                    if (res == SUCCESS)
                    {
                        parse.states.push_back(state);
                    }
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
        ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
        LocalElementParse elemParse = {};

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Check that element type is valid.
        if (ElementTypeInfo::fromName.find(kIt.str())
            == ElementTypeInfo::fromName.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "unknown type `" + kIt.str() + "`");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokType, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Check that element name is not reserved.
        if (ConfigUtil::reserved.find(kIt.str()) != ConfigUtil::reserved.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText, ss.str());
            return E_SMP_NAME_DUPE;
        }

        // Take element name.
        elemParse.tokName = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokName, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token is an assignment operator.
        if ((kIt.type() != Token::OPERATOR) || (kIt.str() != "="))
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected `=` after element name");
            return E_SMP_LOC_OP;
        }

        // Take assignment operator.
        const Token& tokAsgOp = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, tokAsgOp, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be the element initial value,
        // is a constant.
        if (kIt.type() != Token::CONSTANT)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
                    ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                         "redundant read-only annotation");
                    return E_SMP_RO_MULT;
                }

                elemParse.readOnly = true;
                kIt.take();
            }
            else
            {
                // Unknown annotation.
                ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
        ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
        StateVectorElementParse elemParse = {};

        // Check that current token, which should be the element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Check that element type is valid.
        auto typeInfoIt = ElementTypeInfo::fromName.find(kIt.str());
        if (typeInfoIt == ElementTypeInfo::fromName.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "unknown type `" + kIt.str() + "`");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokType, errText, kConfigErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be the element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Check that element name is not reserved.
        if (ConfigUtil::reserved.find(kIt.str()) != ConfigUtil::reserved.end())
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText, ss.str());
            return E_SMP_NAME_DUPE;
        }

        // Check that element exists in the state vector config.
        const IElement* elemObj = nullptr;
        if (kSv.getIElement(kIt.str().c_str(), elemObj) != SUCCESS)
        {
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "state vector does not contain an element"
                                 " named `" + kIt.str() + "`");
            return E_SMP_SV_NAME;
        }

        // Check that element has the same type in the state vector config.
        const ElementTypeInfo& elemTypeInfo = (*typeInfoIt).second;
        if (elemTypeInfo.enumVal != elemObj->type())
        {
            auto it = ElementTypeInfo::fromEnum.find(elemObj->type());
            SFA_ASSERT(it != ElementTypeInfo::fromEnum.end());
            std::stringstream ss;
            ss << "element `" << kIt.str() << "` is type `" << (*it).second.name
               << "` in the state vector config but `" << elemParse.tokType.str
               << "` here";
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText, ss.str());
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
                    ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
                    ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
                        kConfigErr, kIt.tok(), errText, ss.str());
                    return E_SMP_NAME_DUPE;
                }

                // Take alias.
                elemParse.tokAlias = kIt.take();
                elemParse.alias = match[1].str();
            }
            else
            {
                // Unknown annotation.
                ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
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
            // messages from the state machine parser.
            kConfigErr->text = errText;
        }
        return res;
    }

    return parseImpl(toks, kSv, kConfig, kConfigErr);
}
