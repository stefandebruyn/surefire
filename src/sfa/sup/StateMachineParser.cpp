#include <regex>

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

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(kConfigErr, tok, errText,
                                 "expected assignment after element name");
            return E_SMP_ACT_ELEM;
        }

        // Take assignment operator.
        const Token& tokEq = kIt.take();
        if ((tokEq.type != Token::OPERATOR) || (tokEq.str != "="))
        {
            ConfigUtil::setError(kConfigErr, tokEq, errText,
                                 "expected assignment operator");
            return E_SMP_ACT_OP;
        }

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(
                kConfigErr, tokEq, errText,
                "expected expression after assignment operator");
            return E_SMP_ACT_EXPR;
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
            ConfigUtil::setError(kConfigErr, tok, errText,
                                 "unexpected operator");
            return E_SMP_TR_OP;
        }

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(kConfigErr, tok, errText,
                                 "expected destination state after `->`");
            return E_SMP_TR_DEST;
        }

        if (kIt.type() != Token::IDENTIFIER)
        {
            // Unexpected token after transition operator.
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                 "expected destination state after `->`");
            return E_SMP_TR_TOK;
        }

        // Take destination state token.
        kAction->tokDestState = kIt.take();

        if (!kIt.eof())
        {
            // Unexpected token after destination state.
            ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                "unexpected token after transition");
            return E_SMP_TR_JUNK;
        }
    }
    else
    {
        // Unexpected token in action.
        ConfigUtil::setError(kConfigErr, tok, errText,
                             "expected element name or `->`");
        return E_SMP_ACT_TOK;
    }

    return SUCCESS;
}

Result StateMachineParser::parseBlock(TokenIterator kIt,
                                      std::shared_ptr<BlockParse>& kBlock,
                                      ConfigErrorInfo* kConfigErr)
{
    std::shared_ptr<BlockParse> block;
    std::shared_ptr<BlockParse> firstBlock;
    Result res = SUCCESS;

    while (!kIt.eof())
    {
        if (block == nullptr)
        {
            // Allocate first block on first iteration of this loop. This causes
            // an empty label to result in a null block pointer, as if the label
            // wasn't there at all.
            block.reset(new BlockParse{});
            firstBlock = block;
        }

        // Find end index of next thing to parse. Which thing it is will depend
        // on what the end token is.
        U32 idxEnd = kIt.next({Token::COLON, Token::LBRACE, Token::NEWLINE});

        // Determine if the next thing is a guard.
        bool isGuard = false;
        if ((idxEnd == kIt.size()) || (kIt[idxEnd].type == Token::NEWLINE))
        {
            // If there's a newline at the end index, check if the next
            // non-newline token is a left brace. If it is, then we'll try to
            // parse a guard. This allows the user to put the left brace
            // following a guard on the next line if they prefer that style.
            const U32 idxSave = kIt.idx();
            kIt.seek(idxEnd);
            kIt.eat();
            if (kIt.type() == Token::LBRACE)
            {
                isGuard = true;
                idxEnd = kIt.idx();
            }
            kIt.seek(idxSave);
        }
        else
        {
            // Next thing must be a guard since there's a colon or left brace at
            // the end index.
            isGuard = true;
        }

        if (isGuard)
        {
            // Parse guarded action or block of actions.

            // Take optional if.
            if (kIt.str() == "IF")
            {
                kIt.take();
            }

            // Check that guard expression contains at least 1 token.
            if (kIt.idx() >= idxEnd)
            {
                ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                     "expected guard");
                return E_SMP_GUARD;
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
            kIt.eat();

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
                    ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                         "unbalanced brace");
                    return E_SMP_BRACE;
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
                kIt.take();

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
                        ConfigUtil::setError(kConfigErr, kIt.tok(), errText,
                                            "unbalanced brace");
                        return E_SMP_BRACE;
                    }
                }
                else
                {
                    // Guard is followed by a colon, so find the next newline.
                    idxElseEnd = kIt.next({Token::NEWLINE});
                }

                // Take left brace or colon following else.
                const Token& tokAfterElse = kIt.take();

                // Check that else branch contains at least 1 token.
                if (kIt.idx() >= idxElseEnd)
                {
                    ConfigUtil::setError(kConfigErr, tokAfterElse, errText,
                                         "expected logic after else");
                    return E_SMP_ELSE;
                }

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
        else
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

Result StateMachineParser::parseStateSection(TokenIterator& kIt,
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
            if (kConfigErr != nullptr)
            {
                ConfigUtil::setError(kConfigErr, tokLab, errText,
                                     "expected label");
            }
            return E_SMP_LAB;
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
            if (kState.entry != nullptr)
            {
                // Multiple entry labels.
                SFA_ASSERT(false);
            }
            kState.entry = label;
        }
        else if (tokLab.str == ".STEP")
        {
            if (kState.step != nullptr)
            {
                // Multiple entry labels.
                SFA_ASSERT(false);
            }
            kState.step = label;
        }
        else if (tokLab.str == ".EXIT")
        {
            if (kState.exit != nullptr)
            {
                // Multiple entry labels.
                SFA_ASSERT(false);
            }
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
                                                   Parse& kParse,
                                                   ConfigErrorInfo* kConfigErr)
{
    // Check that a state vector section has not already been parsed.
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

Result StateMachineParser::parse(const std::vector<Token>& kToks,
                                 Parse& kParse,
                                 ConfigErrorInfo* kConfigErr)
{
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
                    // State vector section.
                    res = parseStateVectorSection(it, parse, kConfigErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                }
                else if (it.str() == "[LOCAL]")
                {
                    // Local elements section.
                    res = parseLocalSection(it, parse, kConfigErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                }
                else
                {
                    // State section.
                    StateParse state = {};
                    res = parseStateSection(it, state, kConfigErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                    parse.states.push_back(state);
                }
                break;

            default:
                // Unexpected token.
                ConfigUtil::setError(kConfigErr, it.tok(), errText,
                                     "unexpected token");
                return E_SMP_TOK;
        }
    }

    kParse = parse;

    return SUCCESS;
}
