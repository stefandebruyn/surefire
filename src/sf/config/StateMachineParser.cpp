#include <regex>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionParser.hpp"
#include "sf/config/StateMachineParser.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "state machine config error";

const std::regex aliasRegex("@ALIAS=([a-zA-Z][a-zA-Z0-9_]*)");

Result parseAction(TokenIterator kIt,
                   Ref<StateMachineParser::ActionParse>& kAction,
                   ErrorInfo* const kErr)
{
    StateMachineParser::ActionParse action{};

    const Token& tok = kIt.take();
    if (tok.type == Token::IDENTIFIER)
    {
        action.tokRhs = tok;

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(kErr, tok, errText,
                                 "expected assignment after element name");
            return E_SMP_ACT_ELEM;
        }

        // Take assignment operator.
        const Token& tokEq = kIt.take();
        if ((tokEq.type != Token::OPERATOR) || (tokEq.str != "="))
        {
            ConfigUtil::setError(kErr, tokEq, errText,
                                 "expected assignment operator");
            return E_SMP_ACT_OP;
        }

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(
                kErr, tokEq, errText,
                "expected expression after assignment operator");
            return E_SMP_ACT_EXPR;
        }

        // Parse expression after assignment operator.
        const Result res = ExpressionParser::parse(
            kIt.slice(kIt.idx(), kIt.size()), action.lhs, kErr);
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
            ConfigUtil::setError(kErr, tok, errText,
                                 "unexpected operator");
            return E_SMP_TR_OP;
        }

        // Check that tokens remain.
        if (kIt.eof())
        {
            ConfigUtil::setError(kErr, tok, errText,
                                 "expected destination state after `->`");
            return E_SMP_TR_DEST;
        }

        if (kIt.type() != Token::IDENTIFIER)
        {
            // Unexpected token after transition operator.
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected destination state after `->`");
            return E_SMP_TR_TOK;
        }

        // Take destination state token.
        action.tokDestState = kIt.take();

        if (!kIt.eof())
        {
            // Unexpected token after destination state.
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                "unexpected token after transition");
            return E_SMP_TR_JUNK;
        }
    }
    else
    {
        // Unexpected token in action.
        ConfigUtil::setError(kErr, tok, errText,
                             "expected element name or `->`");
        return E_SMP_ACT_TOK;
    }

    kAction.reset(new StateMachineParser::ActionParse(action));

    return SUCCESS;
}

} // Anonymous namespace

/////////////////////////////////// Public /////////////////////////////////////

Result StateMachineParser::parseStateSection(
    TokenIterator& kIt,
    StateMachineParser::StateParse& kState,
    ErrorInfo* const kErr)
{
    // Assert that iterator is currently positioned at a section.
    SF_ASSERT(kIt.type() == Token::SECTION);

    // Take section token.
    kState.tokName = kIt.take();

    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        // Take label token.
        const Token& tokLab = kIt.take();
        if (tokLab.type != Token::LABEL)
        {
            if (kErr != nullptr)
            {
                ConfigUtil::setError(kErr, tokLab, errText,
                                     "expected label");
            }
            return E_SMP_LAB;
        }

        // End index of label is the next label or section token (or EOF).
        const U32 idxLabelEnd = kIt.next({Token::LABEL, Token::SECTION});

        // Parse label block.
        Ref<StateMachineParser::BlockParse> label;
        const Result res = StateMachineParser::parseBlock(
            kIt.slice(kIt.idx(), idxLabelEnd),
            label,
            kErr);
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
                SF_ASSERT(false);
            }
            kState.entry = label;
        }
        else if (tokLab.str == ".STEP")
        {
            if (kState.step != nullptr)
            {
                // Multiple entry labels.
                SF_ASSERT(false);
            }
            kState.step = label;
        }
        else if (tokLab.str == ".EXIT")
        {
            if (kState.exit != nullptr)
            {
                // Multiple entry labels.
                SF_ASSERT(false);
            }
            kState.exit = label;
        }
        else
        {
            // Unknown label.
            SF_ASSERT(false);
        }

        // Jump to end of label block.
        kIt.seek(idxLabelEnd);
    }

    return SUCCESS;
}

Result StateMachineParser::parseLocalSection(
    TokenIterator& kIt,
    StateMachineParser::Parse& kParse,
    ErrorInfo* const kErr)
{
    // Check that a local section has not already been parsed.
    if (kParse.hasLocalSection)
    {
        ConfigUtil::setError(kErr, kIt.tok(), errText,
                             "more than one local section");
        return E_SMP_LOC_MULT;
    }
    kParse.hasLocalSection = true;

    // Assert that iterator is currently positioned at the local section.
    SF_ASSERT((kIt.type() == Token::SECTION) && (kIt.str() == "[LOCAL]"));

    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        StateMachineParser::LocalElementParse elemParse = {};

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokType, errText, kErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Take element name.
        elemParse.tokName = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokName, errText, kErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token is an assignment operator.
        if ((kIt.type() != Token::OPERATOR) || (kIt.str() != "="))
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected `=` after element name");
            return E_SMP_LOC_OP;
        }

        // Take assignment operator.
        const Token& tokAsgOp = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, tokAsgOp, errText, kErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be the element initial value,
        // is a constant.
        if (kIt.type() != Token::CONSTANT)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
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
                    ConfigUtil::setError(kErr, kIt.tok(), errText,
                                         "redundant read-only annotation");
                    return E_SMP_RO_MULT;
                }

                elemParse.readOnly = true;
                kIt.take();
            }
            else
            {
                // Unknown annotation.
                ConfigUtil::setError(kErr, kIt.tok(), errText,
                                     "unknown annotation");
                return E_SMP_ANNOT;
            }
        }

        // Add element to parse.
        kParse.localElems.push_back(elemParse);
    }

    return SUCCESS;
}

Result StateMachineParser::parseStateVectorSection(
    TokenIterator& kIt,
    StateMachineParser::Parse& kParse,
    ErrorInfo* const kErr)
{
    // Check that a state vector section has not already been parsed.
    if (kParse.hasStateVectorSection)
    {
        ConfigUtil::setError(kErr, kIt.tok(), errText,
                             "more than one state vector section");
        return E_SMP_SV_MULT;
    }
    kParse.hasStateVectorSection = true;

    // Assert that iterator is currently positioned at the state vector section.
    SF_ASSERT((kIt.type() == Token::SECTION)
               && (kIt.str() == "[STATE_VECTOR]"));

    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        StateMachineParser::StateVectorElementParse elemParse = {};

        // Check that current token, which should be the element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
                                 "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that end of file has not been reached.
        if (ConfigUtil::checkEof(kIt, elemParse.tokType, errText, kErr))
        {
            return E_SMP_EOF;
        }

        // Check that current token, which should be the element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ConfigUtil::setError(kErr, kIt.tok(), errText,
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
                    ConfigUtil::setError(kErr, kIt.tok(), errText,
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
                    ConfigUtil::setError(kErr, kIt.tok(), errText,
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
                ConfigUtil::setError(kErr, kIt.tok(), errText,
                                     "unknown annotation");
                return E_SMP_ANNOT;
            }
        }

        // Add element to parse.
        kParse.svElems.push_back(elemParse);
    }

    return SUCCESS;
}

Result StateMachineParser::parse(const Vec<Token>& kToks,
                                 StateMachineParser::Parse& kParse,
                                 ErrorInfo* const kErr)
{
    TokenIterator it(kToks.begin(), kToks.end());
    StateMachineParser::Parse parse = {};

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
                    res = parseStateVectorSection(it, parse, kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                }
                else if (it.str() == "[LOCAL]")
                {
                    // Local elements section.
                    res = parseLocalSection(it, parse, kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                }
                else
                {
                    // State section.
                    StateMachineParser::StateParse state = {};
                    res = parseStateSection(it, state, kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }
                    parse.states.push_back(state);
                }
                break;

            default:
                // Unexpected token.
                ConfigUtil::setError(kErr, it.tok(), errText,
                                     "unexpected token");
                return E_SMP_TOK;
        }
    }

    kParse = parse;

    return SUCCESS;
}

Result StateMachineParser::parseBlock(
    TokenIterator kIt,
    Ref<StateMachineParser::BlockParse>& kBlock,
    ErrorInfo* const kErr)
{
    Ref<StateMachineParser::BlockParse> block;
    Ref<StateMachineParser::BlockParse> firstBlock;
    Result res = SUCCESS;

    while (!kIt.eof())
    {
        if (block == nullptr)
        {
            // Allocate first block on first iteration of this loop. This causes
            // an empty label to result in a null block pointer, as if the label
            // wasn't there at all.
            block.reset(new StateMachineParser::BlockParse{});
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
                ConfigUtil::setError(kErr, kIt.tok(), errText,
                                     "expected guard");
                return E_SMP_GUARD;
            }

            // Parse guard.
            res = ExpressionParser::parse(kIt.slice(kIt.idx(), idxEnd),
                                          block->guard,
                                          kErr);
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
                    ConfigUtil::setError(kErr, kIt.tok(), errText,
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
            Result res = StateMachineParser::parseBlock(
                kIt.slice(kIt.idx(), idxBlockEnd),
                block->ifBlock,
                kErr);
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
                        ConfigUtil::setError(kErr, kIt.tok(), errText,
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
                    ConfigUtil::setError(kErr, tokAfterElse, errText,
                                         "expected logic after else");
                    return E_SMP_ELSE;
                }

                // Parse else branch.
                res = StateMachineParser::parseBlock(
                    kIt.slice(kIt.idx(), idxElseEnd),
                    block->elseBlock,
                    kErr);
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
            // If the next token is an assertion annotation, then this is an
            // assertion annotation in a state script.
            if (kIt.type() == Token::ANNOTATION && (kIt.str() == "@ASSERT"))
            {
                // Take annotation.
                kIt.take();

                // Parse assertion expression.
                res = ExpressionParser::parse(kIt.slice(kIt.idx(), idxEnd),
                                              block->assertion,
                                              kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
            }
            else
            {
                // Not an assertion, so an unguarded action.
                res = parseAction(kIt.slice(kIt.idx(), idxEnd),
                                block->action,
                                kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
            }

            // Jump to end of thing just parsed.
            kIt.seek(idxEnd);
            kIt.eat();
        }

        if (!kIt.eof())
        {
            // Add another block in the chain.
            block->next.reset(new StateMachineParser::BlockParse{});
            block = block->next;
        }
    }

    kBlock = firstBlock;

    return SUCCESS;
}
