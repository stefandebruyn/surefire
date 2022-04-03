#include <cstdlib>
#include <cmath>

#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/LanguageConstants.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

static const char* const gErrText = "expression error";

/////////////////////////////////// Public /////////////////////////////////////

Result ExpressionCompiler::compile(const Ref<const ExpressionParse> kParse,
                                   const Map<String, IElement*> kBindings,
                                   const ElementType kEvalType,
                                   Ref<const ExpressionAssembly>& kAsm,
                                   ErrorInfo* const kErr)
{
    // Check that expression parse is non-null.
    if (kParse == nullptr)
    {
        return E_EXC_NULL;
    }

    // Compile expression starting at root.
    ExpressionAssembly::Workspace ws{};
    Ref<IExprNode<F64>> root = nullptr;
    const Result res = ExpressionCompiler::compileImpl(kParse,
                                                       kBindings,
                                                       root,
                                                       ws,
                                                       kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Add cast to target evaluation type. We do this even when both types are
    // F64 so that NaNs can be eliminated by safe-casting.
    Ref<IExpression> newRoot = nullptr;
    SF_SAFE_ASSERT(root != nullptr);
    SF_SAFE_ASSERT(root->type() == ElementType::FLOAT64);
    switch (kEvalType)
    {
        case ElementType::INT8:
            newRoot.reset(new UnaryOpExprNode<I8, F64>(safeCast<I8, F64>,
                                                       *root));
            break;

        case ElementType::INT16:
            newRoot.reset(new UnaryOpExprNode<I16, F64>(safeCast<I16, F64>,
                                                        *root));
            break;

        case ElementType::INT32:
            newRoot.reset(new UnaryOpExprNode<I32, F64>(safeCast<I32, F64>,
                                                        *root));
            break;

        case ElementType::INT64:
            newRoot.reset(new UnaryOpExprNode<I64, F64>(safeCast<I64, F64>,
                                                        *root));
            break;

        case ElementType::UINT8:
            newRoot.reset(new UnaryOpExprNode<U8, F64>(safeCast<U8, F64>,
                                                       *root));
            break;

        case ElementType::UINT16:
            newRoot.reset(new UnaryOpExprNode<U16, F64>(safeCast<U16, F64>,
                                                        *root));
            break;

        case ElementType::UINT32:
            newRoot.reset(new UnaryOpExprNode<U32, F64>(safeCast<U32, F64>,
                                                        *root));
            break;

        case ElementType::UINT64:
            newRoot.reset(new UnaryOpExprNode<U64, F64>(safeCast<U64, F64>,
                                                        *root));
            break;

        case ElementType::FLOAT32:
            newRoot.reset(new UnaryOpExprNode<F32, F64>(safeCast<F32, F64>,
                                                        *root));
            break;

        case ElementType::FLOAT64:
            newRoot.reset(new UnaryOpExprNode<F64, F64>(safeCast<F64, F64>,
                                                        *root));
            break;

        case ElementType::BOOL:
            newRoot.reset(new UnaryOpExprNode<bool, F64>(safeCast<bool, F64>,
                                                         *root));
            break;

        default:
            // Unreachable.
            SF_SAFE_ASSERT(false);
    }

    // Add root node to workspace.
    ws.exprNodes.push_back(newRoot);
    ws.rootNode = newRoot;

    // Create the final assembly.
    kAsm.reset(new ExpressionAssembly(ws));

    return SUCCESS;
}

Ref<IExpression> ExpressionAssembly::root() const
{
    return mWs.rootNode;
}

Vec<Ref<IExpressionStats>> ExpressionAssembly::stats() const
{
    return mWs.exprStats;
}

/////////////////////////////////// Private ////////////////////////////////////

Result ExpressionCompiler::tokenToF64(const Token& kTok,
                                      F64& kRet,
                                      ErrorInfo* const kErr)
{
    // Convert string to F64.
    const char* const str = kTok.str.c_str();
    char* end = nullptr;
    const F64 val = std::strtod(str, &end);

    if (end == str)
    {
        // Invalid numeric constant.
        ErrorInfo::set(kErr, kTok, gErrText, "invalid number");
        return E_EXC_NUM;
    }

    if (val == HUGE_VAL)
    {
        // Numeric constant is out of range.
        ErrorInfo::set(kErr, kTok, gErrText,
                       "number is outside the representable range");
        return E_EXC_OVFL;
    }

    // Success- return converted value.
    kRet = val;

    return SUCCESS;
}

Result ExpressionCompiler::compileStatsFunc(
    const Ref<const ExpressionParse> kParse,
    const Map<String, IElement*>& kBindings,
    Ref<IExprNode<F64>>& kNode,
    ExpressionAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    // Collect argument expression nodes.
    Vec<Ref<const ExpressionParse>> argNodes;
    Ref<const ExpressionParse> node = kParse;
    while (node->left != nullptr)
    {
        argNodes.push_back(node->left);
        node = node->left;
    }

    // Check function arity.
    if (argNodes.size() != 2)
    {
        std::stringstream ss;
        ss << "`" << kParse->data.str << + "` expects 2 arguments, got "
           << argNodes.size();
        ErrorInfo::set(kErr, kParse->data, gErrText, ss.str());
        return E_EXC_ARITY;
    }

    // Compile first argument expression; the expression which stats are being
    // calculated for.
    Ref<IExprNode<F64>> arg1Node = nullptr;
    Result res = ExpressionCompiler::compileImpl(argNodes[0]->right,
                                                 kBindings,
                                                 arg1Node,
                                                 kWs,
                                                 kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile second argument expression, the rolling window size. This one
    // gets passed through the entire compilation process so that we can
    // evaluate it here and get a constant value for the window size.
    Ref<const ExpressionAssembly> arg2Asm;
    res = ExpressionCompiler::compile(argNodes[1]->right,
                                      kBindings,
                                      ElementType::FLOAT64,
                                      arg2Asm,
                                      kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Validate the window size.
    SF_SAFE_ASSERT(arg2Asm != nullptr);
    SF_SAFE_ASSERT(arg2Asm->root() != nullptr);
    SF_SAFE_ASSERT(arg2Asm->root()->type() == ElementType::FLOAT64);
    const F64 windowSizeFp =
        dynamic_cast<IExprNode<F64>*>(arg2Asm->root().get())->evaluate();
    if ((windowSizeFp != windowSizeFp)                // NaN
        || (windowSizeFp <= 0)                        // Negative or zero
        || (std::ceil(windowSizeFp) != windowSizeFp)) // Non-integer
    {
        ErrorInfo::set(kErr, argNodes[1]->right->data, gErrText,
                       "rolling window size must be an integer > 0");
        return E_EXC_WIN;
    }

    // Enforce maximum window size.
    const U32 windowSize = safeCast<U32, F64>(windowSizeFp);
    if (windowSize > LangConst::rollWindowMaxSize)
    {
        std::stringstream ss;
        ss << "rolling window size must be <= " << LangConst::rollWindowMaxSize;
        ErrorInfo::set(kErr, argNodes[1]->right->data, gErrText, ss.str());
        return E_EXC_WIN;
    }

    // Allocate storage arrays needed by expression stats and add them to the
    // workspace.
    const U32 statsArrSizeBytes = (windowSize * sizeof(F64));
    Ref<Vec<U8>> statsArrA(new Vec<U8>(statsArrSizeBytes));
    Ref<Vec<U8>> statsArrB(new Vec<U8>(statsArrSizeBytes));
    kWs.statArrs.push_back(statsArrA);
    kWs.statArrs.push_back(statsArrB);

    // Create expression stats for first argument expression and add it to the
    // workspace. The expression stats is given raw pointers to the arrays we
    // just allocated.
    SF_SAFE_ASSERT(arg1Node != nullptr);
    const Ref<ExpressionStats<F64>> exprStats(
        new ExpressionStats<F64>(*arg1Node,
                                 reinterpret_cast<F64*>(statsArrA->data()),
                                 reinterpret_cast<F64*>(statsArrB->data()),
                                 windowSize));
    kWs.exprStats.push_back(exprStats);

    // Create node which returns the desired stat.
    if (kParse->data.str == LangConst::funcRollAvg)
    {
        kNode.reset(new RollAvgNode(*exprStats));
    }
    else if (kParse->data.str == LangConst::funcRollMedian)
    {
        kNode.reset(new RollMedianNode(*exprStats));
    }
    else if (kParse->data.str == LangConst::funcRollMin)
    {
        kNode.reset(new RollMinNode(*exprStats));
    }
    else if (kParse->data.str == LangConst::funcRollMax)
    {
        kNode.reset(new RollMaxNode(*exprStats));
    }
    else
    {
        kNode.reset(new RollRangeNode(*exprStats));
    }

    // Add compiled function node to workspace.
    kWs.exprNodes.push_back(kNode);

    return SUCCESS;
}

Result ExpressionCompiler::compileFunction(
    const Ref<const ExpressionParse> kParse,
    const Map<String, IElement*>& kBindings,
    Ref<IExprNode<F64>>& kNode,
    ExpressionAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    if ((kParse->data.str == LangConst::funcRollAvg)
        || (kParse->data.str == LangConst::funcRollMedian)
        || (kParse->data.str == LangConst::funcRollMin)
        || (kParse->data.str == LangConst::funcRollMax)
        || (kParse->data.str == LangConst::funcRollRange))
    {
        // Compile expression stats function.
        return ExpressionCompiler::compileStatsFunc(kParse,
                                                        kBindings,
                                                        kNode,
                                                        kWs,
                                                        kErr);
    }

    // Other functions may be added by chaining off the above `if`!

    // If we got this far, the function is not recognized.
    ErrorInfo::set(kErr, kParse->data, gErrText,
                   ("unknown function `" + kParse->data.str + "`"));
    return E_EXC_FUNC;
}

Result ExpressionCompiler::compileOperator(
    const Ref<const ExpressionParse> kParse,
    const Map<String, IElement*>& kBindings,
    Ref<IExprNode<F64>>& kNode,
    ExpressionAssembly::Workspace& kWs,
    ErrorInfo* const kErr)
{
    SF_SAFE_ASSERT(kParse != nullptr);

    // Get operator info.
    SF_SAFE_ASSERT(kParse->data.opInfo != nullptr);
    const OpInfo& opInfo = *kParse->data.opInfo;

    // Compile right subtree.
    Ref<IExprNode<F64>> nodeRight;
    Result res = ExpressionCompiler::compileImpl(kParse->right,
                                                 kBindings,
                                                 nodeRight,
                                                 kWs,
                                                 kErr);
    if (res != SUCCESS)
    {
        return res;
    }
    SF_SAFE_ASSERT(nodeRight != nullptr);

    // If a binary operator, compile left subtree.
    Ref<IExprNode<F64>> nodeLeft;
    if (!opInfo.unary)
    {
        res = ExpressionCompiler::compileImpl(kParse->left,
                                              kBindings,
                                              nodeLeft,
                                              kWs,
                                              kErr);
        if (res != SUCCESS)
        {
            return res;
        }
        SF_SAFE_ASSERT(nodeLeft != nullptr);
    }

    // Create operator node.
    switch (opInfo.enumVal)
    {
        case OpInfo::Type::NOT:
        {
            kNode.reset(new UnaryOpExprNode<F64>(lnot<F64>, *nodeRight));
            break;
        }

        case OpInfo::Type::MULT:
            kNode.reset(new BinOpExprNode<F64>(mult<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;

        case OpInfo::Type::DIV:
            kNode.reset(new BinOpExprNode<F64>(div<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;

        case OpInfo::Type::ADD:
            kNode.reset(new BinOpExprNode<F64>(add<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;

        case OpInfo::Type::SUB:
            kNode.reset(new BinOpExprNode<F64>(sub<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;

        case OpInfo::Type::LT:
        {
            kNode.reset(new BinOpExprNode<F64>(lt<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::LTE:
        {
            kNode.reset(new BinOpExprNode<F64>(lte<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::GT:
        {
            kNode.reset(new BinOpExprNode<F64>(gt<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::GTE:
        {
            kNode.reset(new BinOpExprNode<F64>(gte<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::EQ:
        {
            kNode.reset(new BinOpExprNode<F64>(eq<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::NEQ:
        {
            kNode.reset(new BinOpExprNode<F64>(neq<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::AND:
        {
            kNode.reset(new BinOpExprNode<F64>(land<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        case OpInfo::Type::OR:
        {
            kNode.reset(new BinOpExprNode<F64>(lor<F64>,
                                               *nodeLeft,
                                               *nodeRight));
            break;
        }

        default:
            // Unreachable.
            SF_SAFE_ASSERT(false);
    }

    // Add compiled node to workspace.
    kWs.exprNodes.push_back(kNode);

    return SUCCESS;
}

Result ExpressionCompiler::compileImpl(const Ref<const ExpressionParse> kParse,
                                       const Map<String, IElement*>& kBindings,
                                       Ref<IExprNode<F64>>& kNode,
                                       ExpressionAssembly::Workspace& kWs,
                                       ErrorInfo* const kErr)
{
    // Base case: parse is null, so we fell off the tree.
    if (kParse == nullptr)
    {
        return SUCCESS;
    }

    if (kParse->func)
    {
        // Expression node is a function call.
        return ExpressionCompiler::compileFunction(kParse,
                                                   kBindings,
                                                   kNode,
                                                   kWs,
                                                   kErr);
    }
    else if (kParse->data.type == Token::CONSTANT)
    {
        // Expression node is a constant value.

        // Assert that node is a leaf.
        SF_SAFE_ASSERT(kParse->left == nullptr);
        SF_SAFE_ASSERT(kParse->right == nullptr);

        if (kParse->data.str == LangConst::constantTrue)
        {
            // True boolean constant.
            kNode.reset(new ConstExprNode<F64>(1.0));
        }
        else if (kParse->data.str == LangConst::constantFalse)
        {
            // False boolean constant.
            kNode.reset(new ConstExprNode<F64>(0.0));
        }
        else
        {
            // Numeric constant.
            F64 val = 0.0;
            const Result res = ExpressionCompiler::tokenToF64(kParse->data,
                                                              val,
                                                              kErr);
            if (res != SUCCESS)
            {
                return res;
            }

            kNode.reset(new ConstExprNode<F64>(val));
        }

        // Add compiled node to workspace.
        kWs.exprNodes.push_back(kNode);
    }
    else if (kParse->data.type == Token::IDENTIFIER)
    {
        // Expression node is a state vector element.

        // Assert that node is a leaf.
        SF_SAFE_ASSERT(kParse->left == nullptr);
        SF_SAFE_ASSERT(kParse->right == nullptr);

        // Look up element bound to identifier.
        auto elemIt = kBindings.find(kParse->data.str);
        if (elemIt == kBindings.end())
        {
            ErrorInfo::set(kErr, kParse->data, gErrText, "unknown element");
            return E_EXC_ELEM;
        }
        IElement* const elemObj = (*elemIt).second;

        // Check that element is non-null.
        if (elemObj == nullptr)
        {
            return E_EXC_ELEM_NULL;
        }

        // Narrow the element pointer to a template instantiation of the
        // element's type.
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                const Ref<IExprNode<I8>> nodeElem(new ElementExprNode<I8>(
                    *static_cast<const Element<I8>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, I8>(safeCast<F64, I8>,
                                                         *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT16:
            {
                const Ref<IExprNode<I16>> nodeElem(new ElementExprNode<I16>(
                    *static_cast<const Element<I16>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, I16>(safeCast<F64, I16>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT32:
            {
                const Ref<IExprNode<I32>> nodeElem(new ElementExprNode<I32>(
                    *static_cast<const Element<I32>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, I32>(safeCast<F64, I32>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT64:
            {
                const Ref<IExprNode<I64>> nodeElem(new ElementExprNode<I64>(
                    *static_cast<const Element<I64>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, I64>(safeCast<F64, I64>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT8:
            {
                const Ref<IExprNode<U8>> nodeElem(new ElementExprNode<U8>(
                    *static_cast<const Element<U8>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, U8>(safeCast<F64, U8>,
                                                         *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT16:
            {
                const Ref<IExprNode<U16>> nodeElem(new ElementExprNode<U16>(
                    *static_cast<const Element<U16>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, U16>(safeCast<F64, U16>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT32:
            {
                const Ref<IExprNode<U32>> nodeElem(new ElementExprNode<U32>(
                    *static_cast<const Element<U32>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, U32>(safeCast<F64, U32>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT64:
            {
                const Ref<IExprNode<U64>> nodeElem(new ElementExprNode<U64>(
                    *static_cast<const Element<U64>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, U64>(safeCast<F64, U64>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT32:
            {
                const Ref<IExprNode<F32>> nodeElem(new ElementExprNode<F32>(
                    *static_cast<const Element<F32>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, F32>(safeCast<F64, F32>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT64:
            {
                const Ref<IExprNode<F64>> nodeElem(new ElementExprNode<F64>(
                    *static_cast<const Element<F64>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, F64>(safeCast<F64, F64>,
                                                          *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::BOOL:
            {
                const Ref<IExprNode<bool>> nodeElem(new ElementExprNode<bool>(
                    *static_cast<const Element<bool>*>(elemObj)));
                kNode.reset(new UnaryOpExprNode<F64, bool>(safeCast<F64, bool>,
                                                           *nodeElem));
                kWs.exprNodes.push_back(nodeElem);
                break;
            }

            default:
                // Unreachable.
                SF_SAFE_ASSERT(false);
        }

        // Add compiled node to workspace.
        kWs.exprNodes.push_back(kNode);
    }
    else
    {
        // Compile operator expression node.
        const Result res = ExpressionCompiler::compileOperator(kParse,
                                                               kBindings,
                                                               kNode,
                                                               kWs,
                                                               kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

ExpressionAssembly::ExpressionAssembly(
    const ExpressionAssembly::Workspace& kWs) : mWs(kWs)
{
}
