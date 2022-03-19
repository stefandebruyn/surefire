#include <cstdlib>
#include <cmath>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionCompiler.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "expression error";

const char* const rollAvgFuncName = "ROLL_AVG";

const char* const rollMedianFuncName = "ROLL_MEDIAN";

const char* const rollMinFuncName = "ROLL_MIN";

const char* const rollMaxFuncName = "ROLL_MAX";

const char* const rollRangeFuncName = "ROLL_RANGE";

const U32 maxRollWindowSize = 100000;

struct CompilerState
{
    Vec<const IExpression*> exprNodes;
    Vec<IExpressionStats*> exprStats;
    Vec<const char*> statArrs;
};

Result compileImpl(const Ref<const ExpressionParser::Parse> kParse,
                   const Vec<const StateVector*> kSvs,
                   IExprNode<F64>*& kNode,
                   CompilerState& kCompState,
                   ErrorInfo* const kErr);

Result tokenToF64(const Token& kTok, F64& kRet, ErrorInfo* const kErr)
{
    // Convert string to F64.
    const char* const str = kTok.str.c_str();
    char* end = nullptr;
    const double val = std::strtod(str, &end);

    if (end == str)
    {
        // Invalid numeric constant.
        ConfigUtil::setError(kErr, kTok, errText, "invalid number");
        return E_EXC_NUM;
    }

    if (val == HUGE_VAL)
    {
        // Numeric constant is out of range.
        ConfigUtil::setError(kErr, kTok, errText,
                             "number is outside the representable range");
        return E_EXC_OVFL;
    }

    // Success- return converted value.
    kRet = val;
    return SUCCESS;
}

Result compileExprStatsFunc(const Ref<const ExpressionParser::Parse> kParse,
                            const Vec<const StateVector*> kSvs,
                            IExprNode<F64>*& kNode,
                            CompilerState& kCompState,
                            ErrorInfo* const kErr)
{
    SF_ASSERT(kParse != nullptr);

    // Collect argument expression nodes.
    Vec<Ref<const ExpressionParser::Parse>> argNodes;
    Ref<const ExpressionParser::Parse> node = kParse;
    while (node->left != nullptr)
    {
        argNodes.push_back(node->left);
        node = node->left;
    }

    if (argNodes.size() != 2)
    {
        // Wrong arity.
        std::stringstream ss;
        ss << "`" << kParse->data.str << + "` expects 2 arguments, got "
           << argNodes.size();
        ConfigUtil::setError(kErr, kParse->data, errText, ss.str());
        return E_EXC_ARITY;
    }

    // Compile first argument expression; the expression which stats are being
    // calculated for.
    IExprNode<F64>* arg1Node = nullptr;
    Result res = compileImpl(argNodes[0]->right,
                             kSvs,
                             arg1Node,
                             kCompState,
                             kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Compile second argument expression, the rolling window size. This one
    // gets passed through the entire compilation process so that we can
    // evaluate it here and get a constant value for the window size.
    Ref<const ExpressionCompiler::Assembly> arg2Asm;
    res = ExpressionCompiler::compile(argNodes[1]->right,
                                      kSvs,
                                      ElementType::FLOAT64,
                                      arg2Asm,
                                      kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    // Validate the window size.
    const F64 windowSizeFp =
        static_cast<IExprNode<F64>*>(arg2Asm->root())->evaluate();
    if ((windowSizeFp != windowSizeFp)                // NaN
        || (windowSizeFp <= 0)                        // Negative or zero
        || (std::ceil(windowSizeFp) != windowSizeFp)) // Non-integer
    {
        ConfigUtil::setError(kErr, argNodes[1]->right->data, errText,
                             "rolling window size must be an integer > 0");
        return E_EXC_WIN;
    }

    // Enforce maximum window size.
    const U32 windowSize = safeCast<U32, F64>(windowSizeFp);
    if (windowSize > maxRollWindowSize)
    {
        std::stringstream ss;
        ss << "rolling window size must be <= " << maxRollWindowSize;
        ConfigUtil::setError(kErr, argNodes[1]->right->data, errText, ss.str());
        return E_EXC_WIN;
    }

    // Allocate storage arrays needed by expression stats.
    const U32 statsBytes = (windowSize * sizeof(F64));
    char* const statsArrA = new char[statsBytes];
    char* const statsArrB = new char[statsBytes];
    kCompState.statArrs.push_back(statsArrA);
    kCompState.statArrs.push_back(statsArrB);

    // Create expression stats for first argument expression.
    ExpressionStats<F64>* const exprStats =
        new ExpressionStats<F64>(*arg1Node,
                                 reinterpret_cast<F64*>(statsArrA),
                                 reinterpret_cast<F64*>(statsArrB),
                                 windowSize);
    kCompState.exprStats.push_back(exprStats);

    // Create node which returns the desired stat.
    if (kParse->data.str == rollAvgFuncName)
    {
        kNode = new RollAvgNode(*exprStats);
    }
    else if (kParse->data.str == rollMedianFuncName)
    {
        kNode = new RollMedianNode(*exprStats);
    }
    else if (kParse->data.str == rollMinFuncName)
    {
        kNode = new RollMinNode(*exprStats);
    }
    else if (kParse->data.str == rollMaxFuncName)
    {
        kNode = new RollMaxNode(*exprStats);
    }
    else
    {
        kNode = new RollRangeNode(*exprStats);
    }

    kCompState.exprNodes.push_back(kNode);

    return SUCCESS;
}

Result compileFunction(const Ref<const ExpressionParser::Parse> kParse,
                       const Vec<const StateVector*> kSvs,
                       IExprNode<F64>*& kNode,
                       CompilerState& kCompState,
                       ErrorInfo* const kErr)
{
    SF_ASSERT(kParse != nullptr);

    if ((kParse->data.str == rollAvgFuncName)
        || (kParse->data.str == rollMedianFuncName)
        || (kParse->data.str == rollMinFuncName)
        || (kParse->data.str == rollMaxFuncName)
        || (kParse->data.str == rollRangeFuncName))
    {
        // Compile expression stats function.
        return compileExprStatsFunc(kParse, kSvs, kNode, kCompState, kErr);
    }

    // Other functions may be added by chaining off the above `if`!

    // If we got this far, the function is not recognized.
    ConfigUtil::setError(kErr, kParse->data, errText,
                         "unknown function `" + kParse->data.str + "`");
    return E_EXC_FUNC;
}

Result compileOperator(const Ref<const ExpressionParser::Parse> kParse,
                       const Vec<const StateVector*> kSvs,
                       IExprNode<F64>*& kNode,
                       CompilerState& kCompState,
                       ErrorInfo* const kErr)
{
    // Look up operator info.
    auto opInfoIt = OperatorInfo::fromStr.find(kParse->data.str);
    // Assert that lookup succeeded. This is guaranteed by the expression
    // parser.
    SF_ASSERT(opInfoIt != OperatorInfo::fromStr.end());
    const OperatorInfo& opInfo = (*opInfoIt).second;

    // Compile right subtree.
    SF_ASSERT(kParse->right != nullptr);
    IExprNode<F64>* nodeRight = nullptr;
    Result res = compileImpl(kParse->right, kSvs, nodeRight, kCompState, kErr);
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ASSERT(nodeRight != nullptr);

    // If a binary operator, compile left subtree.
    IExprNode<F64>* nodeLeft = nullptr;
    if (!opInfo.unary)
    {
        SF_ASSERT(kParse->left != nullptr);
        res = compileImpl(kParse->left, kSvs, nodeLeft, kCompState, kErr);
        if (res != SUCCESS)
        {
            return res;
        }
        SF_ASSERT(nodeLeft != nullptr);
    }

    // Create operator node.
    switch (opInfo.enumVal)
    {
        case OperatorInfo::Type::NOT:
        {
            kNode = new UnaryOpExprNode<F64>(
                [] (const F64 a) -> F64 { return !a; },
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::MULT:
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a * b); },
                *nodeLeft,
                *nodeRight);
            break;

        case OperatorInfo::Type::DIV:
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a / b); },
                *nodeLeft,
                *nodeRight);
            break;

        case OperatorInfo::Type::ADD:
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a + b); },
                *nodeLeft,
                *nodeRight);
            break;

        case OperatorInfo::Type::SUB:
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a - b); },
                *nodeLeft,
                *nodeRight);
            break;

        case OperatorInfo::Type::LT:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a < b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::LTE:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a <= b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::GT:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a > b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::GTE:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a >= b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::EQ:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a == b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::NEQ:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a != b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::AND:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a && b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        case OperatorInfo::Type::OR:
        {
            kNode = new BinOpExprNode<F64>(
                [] (const F64 a, const F64 b) -> F64 { return (a || b); },
                *nodeLeft,
                *nodeRight);
            break;
        }

        default:
            // Unreachable; would indicate an error in the operator info LUT.
            SF_ASSERT(false);
    }

    // Track the allocated node.
    kCompState.exprNodes.push_back(kNode);

    return SUCCESS;
}

Result compileImpl(const Ref<const ExpressionParser::Parse> kParse,
                   const Vec<const StateVector*> kSvs,
                   IExprNode<F64>*& kNode,
                   CompilerState& kCompState,
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
        return compileFunction(kParse, kSvs, kNode, kCompState, kErr);
    }
    else if (kParse->data.type == Token::CONSTANT)
    {
        // Expression node is a constant element.

        // Assert that node has no left or right subtrees. This is guaranteed by
        // the expression parser.
        SF_ASSERT(kParse->left == nullptr);
        SF_ASSERT(kParse->right == nullptr);

        if (kParse->data.str == "TRUE")
        {
            // True boolean constant.
            kNode = new ConstExprNode<F64>(1.0);
        }
        else if (kParse->data.str == "FALSE")
        {
            // False boolean constant.
            kNode = new ConstExprNode<F64>(0.0);
        }
        else
        {
            // Numeric constant.
            F64 val = 0.0;
            const Result res = tokenToF64(kParse->data, val, kErr);
            if (res != SUCCESS)
            {
                return res;
            }
            kNode = new ConstExprNode<F64>(val);
        }

        kCompState.exprNodes.push_back(kNode);
    }
    else if (kParse->data.type == Token::IDENTIFIER)
    {
        // Expression node is a state vector element.

        // Assert that node has no left or right subtrees. This is guaranteed by
        // the expression parser.
        SF_ASSERT(kParse->left == nullptr);
        SF_ASSERT(kParse->right == nullptr);

        // Look up element in state vector.
        IElement* elemObj = nullptr;
        for (const StateVector* const sv : kSvs)
        {
            if (sv != nullptr)
            {
                const Result res = sv->getIElement(kParse->data.str.c_str(),
                                                   elemObj);
                if (res == SUCCESS)
                {
                    break;
                }
            }
        }
        if (elemObj == nullptr)
        {
            // Unknown element.
            ConfigUtil::setError(kErr, kParse->data, errText,
                                 "unknown element");
            return E_EXC_ELEM;
        }

        // Narrow the element pointer to a template instantiation of the
        // element's type.
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                IExprNode<I8>* const nodeElem = new ElementExprNode<I8>(
                    *static_cast<const Element<I8>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I8>(safeCast<F64, I8>,
                                                     *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT16:
            {
                IExprNode<I16>* const nodeElem = new ElementExprNode<I16>(
                    *static_cast<const Element<I16>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I16>(safeCast<F64, I16>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT32:
            {
                IExprNode<I32>* const nodeElem = new ElementExprNode<I32>(
                    *static_cast<const Element<I32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I32>(safeCast<F64, I32>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT64:
            {
                IExprNode<I64>* const nodeElem = new ElementExprNode<I64>(
                    *static_cast<const Element<I64>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I64>(safeCast<F64, I64>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT8:
            {
                IExprNode<U8>* const nodeElem = new ElementExprNode<U8>(
                    *static_cast<const Element<U8>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U8>(safeCast<F64, U8>,
                                                     *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT16:
            {
                IExprNode<U16>* const nodeElem = new ElementExprNode<U16>(
                    *static_cast<const Element<U16>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U16>(safeCast<F64, U16>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT32:
            {
                IExprNode<U32>* const nodeElem = new ElementExprNode<U32>(
                    *static_cast<const Element<U32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U32>(safeCast<F64, U32>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT64:
            {
                IExprNode<U64>* const nodeElem = new ElementExprNode<U64>(
                    *static_cast<const Element<U64>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U64>(safeCast<F64, U64>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT32:
            {
                IExprNode<F32>* const nodeElem = new ElementExprNode<F32>(
                    *static_cast<const Element<F32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, F32>(safeCast<F64, F32>,
                                                      *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT64:
                kNode = new ElementExprNode<F64>(
                    *static_cast<const Element<F64>*>(elemObj));
                break;

            case ElementType::BOOL:
            {
                IExprNode<bool>* const nodeElem =
                    new ElementExprNode<bool>(
                        *static_cast<const Element<bool>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, bool>(safeCast<F64, bool>,
                                                       *nodeElem);
                kCompState.exprNodes.push_back(nodeElem);
                break;
            }
        }

        // Assert that element type was recognized.
        SF_ASSERT(kNode != nullptr);

        kCompState.exprNodes.push_back(kNode);
    }
    else
    {
        // Compile operator expression node.
        const Result res = compileOperator(kParse,
                                           kSvs,
                                           kNode,
                                           kCompState,
                                           kErr);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

} // Anonymous namespace

/////////////////////////////////// Public /////////////////////////////////////

ExpressionCompiler::Assembly::Assembly(IExpression* const kRoot,
                                       const Vec<const IExpression*>& kNodes,
                                       const Vec<IExpressionStats*>& kStats,
                                       const Vec<const char*>& kStatArrs) :
    mRoot(kRoot), mNodes(kNodes), mStats(kStats), mStatArrs(kStatArrs)
{
}

const Vec<IExpressionStats*>& ExpressionCompiler::Assembly::stats() const
{
    return mStats;
}

ExpressionCompiler::Assembly::~Assembly()
{
    // Delete expression nodes.
    for (const IExpression* const node : mNodes)
    {
        delete node;
    }

    // Delete expression stat arrays.
    for (const char* const arr : mStatArrs)
    {
        delete[] arr;
    }

    // Delete expression stats.
    for (const IExpressionStats* const stats : mStats)
    {
        delete stats;
    }
}

IExpression* ExpressionCompiler::Assembly::root() const
{
    return mRoot;
}

Result ExpressionCompiler::compile(
    const Ref<const ExpressionParser::Parse> kParse,
    const Vec<const StateVector*> kSvs,
    const ElementType kEvalType,
    Ref<const ExpressionCompiler::Assembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Check that expression parse is non-null.
    if (kParse == nullptr)
    {
        return E_EXC_NULL;
    }

    // Compile expression starting at root.
    CompilerState compState = {};
    IExprNode<F64>* root = nullptr;
    const Result res = compileImpl(kParse, kSvs, root, compState, kErr);
    if (res != SUCCESS)
    {
        // Delete all allocations since aborting compilation.
        for (const IExpression* const node : compState.exprNodes)
        {
            delete node;
        }

        for (const char* const arr : compState.statArrs)
        {
            delete[] arr;
        }

        for (const IExpressionStats* const stats : compState.exprStats)
        {
            delete stats;
        }

        return res;
    }

    // Add cast to target evaluation type. We do this even when both types are
    // F64 so that NaNs can be eliminated by `safeCast`.
    IExpression* newRoot = nullptr;
    switch (kEvalType)
    {
        case ElementType::INT8:
            newRoot = new UnaryOpExprNode<I8, F64>(safeCast<I8, F64>, *root);
            break;

        case ElementType::INT16:
            newRoot = new UnaryOpExprNode<I16, F64>(safeCast<I16, F64>, *root);
            break;

        case ElementType::INT32:
            newRoot = new UnaryOpExprNode<I32, F64>(safeCast<I32, F64>, *root);
            break;

        case ElementType::INT64:
            newRoot = new UnaryOpExprNode<I64, F64>(safeCast<I64, F64>, *root);
            break;

        case ElementType::UINT8:
            newRoot = new UnaryOpExprNode<U8, F64>(safeCast<U8, F64>, *root);
            break;

        case ElementType::UINT16:
            newRoot = new UnaryOpExprNode<U16, F64>(safeCast<U16, F64>, *root);
            break;

        case ElementType::UINT32:
            newRoot = new UnaryOpExprNode<U32, F64>(safeCast<U32, F64>, *root);
            break;

        case ElementType::UINT64:
            newRoot = new UnaryOpExprNode<U64, F64>(safeCast<U64, F64>, *root);
            break;

        case ElementType::FLOAT32:
            newRoot = new UnaryOpExprNode<F32, F64>(safeCast<F32, F64>, *root);
            break;

        case ElementType::FLOAT64:
            newRoot = new UnaryOpExprNode<F64, F64>(safeCast<F64, F64>, *root);
            break;

        case ElementType::BOOL:
            newRoot = new UnaryOpExprNode<bool, F64>(safeCast<bool, F64>,
                                                     *root);
            break;

        default:
            SF_ASSERT(false);
    }

    compState.exprNodes.push_back(newRoot);

    // Return compiled expression assembly.
    kAsm.reset(new ExpressionCompiler::Assembly(newRoot,
                                                compState.exprNodes,
                                                compState.exprStats,
                                                compState.statArrs));

    return SUCCESS;
}
