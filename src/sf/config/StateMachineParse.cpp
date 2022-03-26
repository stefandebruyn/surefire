#include <regex>

#include "sf/config/ExpressionParse.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/config/StateMachineParse.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "state machine config error";

/////////////////////////////////// Public /////////////////////////////////////

Result StateMachineParse::parseStateSection(
    TokenIterator& kIt,
    StateMachineParse::StateParse& kParse,
    ErrorInfo* const kErr)
{
    // Assert that iterator is currently positioned at a section.
    SF_SAFE_ASSERT(kIt.type() == Token::SECTION);

    // Take section token.
    kParse.tokName = kIt.take();

    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        // Take label token.
        const Token& tokLab = kIt.take();
        if (tokLab.type != Token::LABEL)
        {
            ErrorInfo::set(kErr, tokLab, gErrText, "expected label");
            return E_SMP_NO_LAB;
        }

        // End index of label is the next label or section token (or EOF).
        const U32 idxLabelEnd = kIt.next({Token::LABEL, Token::SECTION});

        // Parse label block.
        Ref<const StateMachineParse::BlockParse> label;
        const Result res = StateMachineParse::parseBlock(
            kIt.slice(kIt.idx(), idxLabelEnd),
            label,
            kErr);
        if (res != SUCCESS)
        {
            return res;
        }
        SF_SAFE_ASSERT(label != nullptr);

        // Assign label block to state based on label name.
        if (tokLab.str == LangConst::labelEntry)
        {
            // Check that an entry label wasn't already parsed.
            if (kParse.entry != nullptr)
            {
                ErrorInfo::set(kErr, tokLab, gErrText, "multiple entry labels");
                return E_SMP_LAB_DUPE;
            }

            kParse.entry = label;
        }
        else if (tokLab.str == LangConst::labelStep)
        {
            // Check that a step label wasn't already parsed.
            if (kParse.step != nullptr)
            {
                ErrorInfo::set(kErr, tokLab, gErrText, "multiple step labels");
                return E_SMP_LAB_DUPE;
            }

            kParse.step = label;
        }
        else if (tokLab.str == LangConst::labelExit)
        {
            // Check that an exit label wasn't already parsed.
            if (kParse.exit != nullptr)
            {
                ErrorInfo::set(kErr, tokLab, gErrText, "multiple exit labels");
                return E_SMP_LAB_DUPE;
            }

            kParse.exit = label;
        }
        else
        {
            // Unknown label.
            ErrorInfo::set(kErr, tokLab, gErrText,
                           ("unknown label `" + tokLab.str + "`"));
            return E_SMP_LAB;
        }

        // Jump to end of label block.
        kIt.seek(idxLabelEnd);
    }

    return SUCCESS;
}

Result StateMachineParse::parseLocalSection(
    TokenIterator& kIt,
    Vec<StateMachineParse::LocalElementParse>& kParse,
    ErrorInfo* const kErr)
{
    // Assert that iterator is currently positioned at the local section.
    SF_SAFE_ASSERT((kIt.type() == Token::SECTION)
                   && (kIt.str() == LangConst::sectionLocal));

    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        StateMachineParse::LocalElementParse elemParse{};

        // Check that current token, which should be an element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ErrorInfo::set(kErr, kIt.tok(), gErrText, "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that current token, which should be an element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ErrorInfo::set(kErr, elemParse.tokType, gErrText,
                           "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Take element name.
        elemParse.tokName = kIt.take();

        // Check that current token is an assignment operator.
        if ((kIt.type() != Token::OPERATOR) || (kIt.str() != "="))
        {
            ErrorInfo::set(kErr, elemParse.tokName, gErrText,
                           "expected `=` after element name");
            return E_SMP_LOC_OP;
        }

        // Take assignment operator.
        const Token& tokAsgOp = kIt.take();

        // Find end of initial value expression on RHS of assignment operator.
        // It may end with a newline or an annotation attached to the element.
        const U32 idxEnd = kIt.next({Token::NEWLINE, Token::ANNOTATION});

        // Slice a new iterator for the initial value expression and check that
        // it's non-empty.
        TokenIterator rhsIt = kIt.slice(kIt.idx(), idxEnd);
        if (rhsIt.eof())
        {
            ErrorInfo::set(kErr, tokAsgOp, gErrText,
                           "expected element initial value after `=`");
            return E_SMP_LOC_VAL;
        }

        // Parse initial value expression.
        const Result res = ExpressionParse::parse(rhsIt,
                                                  elemParse.initValExpr,
                                                  kErr);
        if (res != SUCCESS)
        {
            return res;
        }

        // Jump to end of initial value expression.
        kIt.seek(idxEnd);
        kIt.eat();

        // Take annotations.
        while (kIt.type() == Token::ANNOTATION)
        {
            if (kIt.str() == LangConst::annotationReadOnly)
            {
                // Read-only annotation.

                // Check that element is not already marked read-only.
                if (elemParse.readOnly)
                {
                    ErrorInfo::set(kErr, kIt.tok(), gErrText,
                                   "redundant read-only annotation");
                    return E_SMP_RO_MULT;
                }

                elemParse.readOnly = true;
                kIt.take();
            }
            else
            {
                // Unknown annotation.
                ErrorInfo::set(kErr, kIt.tok(), gErrText, "unknown annotation");
                return E_SMP_ANNOT;
            }
        }

        // Add element to return vector.
        kParse.push_back(elemParse);
    }

    return SUCCESS;
}

Result StateMachineParse::parseStateVectorSection(
    TokenIterator& kIt,
    Vec<StateMachineParse::StateVectorElementParse>& kParse,
    ErrorInfo* const kErr)
{
    // Assert that iterator is currently positioned at the state vector section.
    SF_SAFE_ASSERT((kIt.type() == Token::SECTION)
                   && (kIt.str() == LangConst::sectionStateVector));

    // Take section token.
    kIt.take();

    // Loop until end of token stream or another section.
    while (!kIt.eof() && (kIt.type() != Token::SECTION))
    {
        StateMachineParse::StateVectorElementParse elemParse{};

        // Check that current token, which should be the element type, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ErrorInfo::set(kErr, kIt.tok(), gErrText, "expected element type");
            return E_SMP_ELEM_TYPE;
        }

        // Take element type.
        elemParse.tokType = kIt.take();

        // Check that current token, which should be the element name, is an
        // identifier.
        if (kIt.type() != Token::IDENTIFIER)
        {
            ErrorInfo::set(kErr, elemParse.tokType, gErrText,
                           "expected element name after type");
            return E_SMP_ELEM_NAME;
        }

        // Take element name.
        elemParse.tokName = kIt.take();

        // Take annotations.
        while (kIt.type() == Token::ANNOTATION)
        {
            std::smatch match;
            if (kIt.str() == LangConst::annotationReadOnly)
            {
                // Read-only annotation.

                // Check that element is not already marked read-only.
                if (elemParse.readOnly)
                {
                    ErrorInfo::set(kErr, kIt.tok(), gErrText,
                                   "redundant read-only annotation");
                    return E_SMP_RO_MULT;
                }

                // Take annotation.
                elemParse.readOnly = true;
                kIt.take();
            }
            else if (kIt.str() == LangConst::annotationAlias)
            {
                // Alias annotation.

                // Check that element is not already aliased.
                if (elemParse.tokAlias.str.size() > 0)
                {
                    ErrorInfo::set(kErr, kIt.tok(), gErrText,
                                   "an element may only have one alias");
                    return E_SMP_AL_MULT;
                }

                // Take alias annotation.
                const Token& tokAnnot = kIt.take();

                // Check that next token, which should be the alias name, is an
                // identifier.
                if (kIt.type() != Token::IDENTIFIER)
                {
                    ErrorInfo::set(kErr, tokAnnot, gErrText,
                                   ("expected alias name after `"
                                    + tokAnnot.str + "`"));
                    return E_SMP_ALIAS;
                }

                // Take alias.
                elemParse.tokAlias = kIt.take();
            }
            else
            {
                // Unknown annotation.
                ErrorInfo::set(kErr, kIt.tok(), gErrText, "unknown annotation");
                return E_SMP_ANNOT;
            }
        }

        // Add element to parse.
        kParse.push_back(elemParse);
    }

    return SUCCESS;
}

Result StateMachineParse::parse(const Vec<Token>& kToks,
                                Ref<const StateMachineParse>& kParse,
                                ErrorInfo* const kErr)
{
    // Create iterator for token vector.
    TokenIterator it(kToks.begin(), kToks.end());

    // Data to be parsed.
    Vec<StateMachineParse::StateVectorElementParse> svElems;
    Vec<StateMachineParse::LocalElementParse> localElems;
    Vec<StateMachineParse::StateParse> states;
    bool hasStateVectorSection = false;
    bool hasLocalSection = false;

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
                    // Check that state vector section was not already parsed.
                    if (hasStateVectorSection)
                    {
                        ErrorInfo::set(kErr, it.tok(), gErrText,
                                       "more than one state vector section");
                        return E_SMP_SV_MULT;
                    }

                    // Parse state vector section.
                    res = StateMachineParse::parseStateVectorSection(it,
                                                                     svElems,
                                                                     kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }

                    hasStateVectorSection = true;
                }
                else if (it.str() == "[LOCAL]")
                {
                    // Check that local section was not already parsed.
                    if (hasLocalSection)
                    {
                        ErrorInfo::set(kErr, it.tok(), gErrText,
                                       "more than one local section");
                        return E_SMP_LOC_MULT;
                    }

                    // Parse local section.
                    res = StateMachineParse::parseLocalSection(it,
                                                               localElems,
                                                               kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }

                    hasLocalSection = true;
                }
                else
                {
                    // Parse  State section.
                    StateMachineParse::StateParse state{};
                    res = StateMachineParse::parseStateSection(it, state, kErr);
                    if (res != SUCCESS)
                    {
                        return res;
                    }

                    states.push_back(state);
                }
                break;

            default:
                // Unexpected token.
                ErrorInfo::set(kErr, it.tok(), gErrText, "unexpected token");
                return E_SMP_TOK;
        }
    }

    // Create final parse.
    kParse.reset(new StateMachineParse(svElems,
                                       localElems,
                                       states,
                                       hasStateVectorSection,
                                       hasLocalSection));

    return SUCCESS;
}

Result StateMachineParse::parseBlock(
    TokenIterator kIt,
    Ref<const StateMachineParse::BlockParse>& kParse,
    ErrorInfo* const kErr)
{
    // Call recursive helper.
    Ref<StateMachineParse::MutBlockParse> mutBlock;
    const Result res = StateMachineParse::parseBlockRec(kIt, mutBlock, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // If a block was parsed, convert block tree to public, const type.
    if (mutBlock != nullptr)
    {
        mutBlock->toBlockParse(kParse);
    }

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

void StateMachineParse::MutBlockParse::toBlockParse(
    Ref<const StateMachineParse::BlockParse>& kParse)
{
    Ref<const StateMachineParse::BlockParse> ifBlock;
    Ref<const StateMachineParse::BlockParse> elseBlock;
    Ref<const StateMachineParse::BlockParse> next;

    // Convert linked blocks.
    if (this->ifBlock != nullptr)
    {
        this->ifBlock->toBlockParse(ifBlock);
    }
    if (this->elseBlock != nullptr)
    {
        this->elseBlock->toBlockParse(elseBlock);
    }
    if (this->next != nullptr)
    {
        this->next->toBlockParse(next);
    }

    // Convert the current block.
    kParse.reset(new StateMachineParse::BlockParse{this->guard,
                                                   this->action,
                                                   ifBlock,
                                                   elseBlock,
                                                   next,
                                                   this->assert,
                                                   this->tokElse,
                                                   this->tokAssert,
                                                   this->tokStop});
}

Result StateMachineParse::parseAction(
    TokenIterator kIt,
    Ref<const StateMachineParse::ActionParse>& kParse,
    ErrorInfo* const kErr)
{
    // Data to parse that will become members of the parsed action.
    Token tokRhs{};
    Ref<const ExpressionParse> lhs;
    Token tokDestState{};
    Token tokTransitionKeyword{};

    // Look at first token to determine action type.
    const Token& tok = kIt.take();

    if (tok.type == Token::IDENTIFIER)
    {
        tokRhs = tok;

        // Check that tokens remain.
        if (kIt.eof())
        {
            ErrorInfo::set(kErr, tok, gErrText,
                           "expected assignment after element name");
            return E_SMP_ACT_ELEM;
        }

        // Take assignment operator.
        const Token& tokEq = kIt.take();
        if ((tokEq.type != Token::OPERATOR) || (tokEq.str != "="))
        {
            ErrorInfo::set(kErr, tokEq, gErrText,
                           "expected assignment operator");
            return E_SMP_ACT_OP;
        }

        // Check that tokens remain.
        if (kIt.eof())
        {
            ErrorInfo::set(kErr, tokEq, gErrText,
                           "expected expression after assignment operator");
            return E_SMP_ACT_EXPR;
        }

        // Parse expression after assignment operator.
        const Result res = ExpressionParse::parse(
            kIt.slice(kIt.idx(), kIt.size()),
            lhs,
            kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }
    else if (tok.str == LangConst::keywordTransition)
    {
        // Parse transition action.

        // Save transition keyword token.
        tokTransitionKeyword = tok;

        // Check that tokens remain.
        if (kIt.eof())
        {
            std::stringstream ss;
            ErrorInfo::set(kErr, tok, gErrText,
                           ("expected destination state after `" + tok.str
                            + "`"));
            return E_SMP_TR_DEST;
        }

        if (kIt.type() != Token::IDENTIFIER)
        {
            // Unexpected token after transition operator.
            ErrorInfo::set(kErr, kIt.tok(), gErrText,
                           ("expected destination state after `" + tok.str
                            + "`"));
            return E_SMP_TR_TOK;
        }

        // Take destination state token.
        tokDestState = kIt.take();

        if (!kIt.eof())
        {
            // Unexpected token after destination state.
            ErrorInfo::set(kErr, kIt.tok(), gErrText,
                           ("unexpected token after `" + tok.str + "`"));
            return E_SMP_TR_JUNK;
        }
    }
    else
    {
        // Unexpected token in action.
        ErrorInfo::set(kErr, tok, gErrText,
                       ("expected element name for assignment or `"
                        + LangConst::keywordTransition + "`"));
        return E_SMP_ACT_TOK;
    }

    // Action is valid- return parse.
    kParse.reset(new StateMachineParse::ActionParse{
        tokRhs,
        lhs,
        tokDestState,
        tokTransitionKeyword});

    return SUCCESS;
}

Result StateMachineParse::parseBlockRec(
    TokenIterator kIt,
    Ref<StateMachineParse::MutBlockParse>& kParse,
    ErrorInfo* const kErr)
{
    Result res = SUCCESS;

    // Data to parse that will become members of the parsed block.
    Ref<StateMachineParse::MutBlockParse> block;
    Ref<StateMachineParse::MutBlockParse> firstBlock;

    // Allocate first block in chain. This is done before entering the parse
    // loop so that an empty label results in an all-null block, which makes
    // detection of duplicate labels easier.
    block.reset(new StateMachineParse::MutBlockParse{});
    firstBlock = block;

    while (!kIt.eof())
    {
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
            if (kIt.str() == LangConst::keywordIf)
            {
                kIt.take();
            }

            // Check that guard expression contains at least 1 token.
            if (kIt.idx() >= idxEnd)
            {
                ErrorInfo::set(kErr, kIt.tok(), gErrText, "expected guard");
                return E_SMP_GUARD;
            }

            // Parse guard.
            res = ExpressionParse::parse(kIt.slice(kIt.idx(), idxEnd),
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
                    ErrorInfo::set(kErr, kIt.tok(), gErrText,
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
            Result res = StateMachineParse::parseBlockRec(
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

            if (kIt.str() == LangConst::keywordElse)
            {
                // Guard has an else branch.

                // Take else token.
                block->tokElse = kIt.take();

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
                        ErrorInfo::set(kErr, kIt.tok(), gErrText,
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
                    ErrorInfo::set(kErr, tokAfterElse, gErrText,
                                   "expected logic after else");
                    return E_SMP_ELSE;
                }

                // Parse else branch.
                res = StateMachineParse::parseBlockRec(
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
            // If the next token is an assert annotation, then it marks an
            // assert expression in a state script.
            if (kIt.str() == LangConst::annotationAssert)
            {
                SF_SAFE_ASSERT(kIt.type() == Token::ANNOTATION);

                // Take assert annotation.
                block->tokAssert = kIt.take();

                // Parse assert expression.
                res = ExpressionParse::parse(kIt.slice(kIt.idx(), idxEnd),
                                             block->assert,
                                             kErr);
                if (res != SUCCESS)
                {
                    return res;
                }
            }
            // If the next token is a stop annotation, then it marks an exit
            // point for a state script.
            else if (kIt.str() == LangConst::annotationStop)
            {
                SF_SAFE_ASSERT(kIt.type() == Token::ANNOTATION);

                // Take stop annotation.
                block->tokStop = kIt.take();
            }
            // Not a state script assert or stop, so an unguarded state machine
            // action or state script input.
            else
            {
                res = StateMachineParse::parseAction(
                    kIt.slice(kIt.idx(), idxEnd),
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
            SF_SAFE_ASSERT(block != nullptr);
            block->next.reset(new StateMachineParse::MutBlockParse{});
            block = block->next;
        }
    }

    // Return the root block.
    kParse = firstBlock;

    return SUCCESS;
}

StateMachineParse::StateMachineParse(
    const Vec<StateMachineParse::StateVectorElementParse>& kSvElems,
    const Vec<StateMachineParse::LocalElementParse>& kLocalElems,
    const Vec<StateMachineParse::StateParse>& kStates,
    const bool kHasStateVectorSection,
    const bool kHasLocalSection) :
    svElems(kSvElems),
    localElems(kLocalElems),
    states(kStates),
    hasStateVectorSection(kHasStateVectorSection),
    hasLocalSection(kHasLocalSection)
{
}
