#include <cstdlib>
#include <cmath>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionCompiler.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "expression error";

Result compileImpl(const Ref<const ExpressionParser::Parse> kParse,
                   const Vec<const StateVector*> kSvs,
                   const IExprNode<F64>*& kNode,
                   Vec<const IExpression*>& kExprNodes,
                   Vec<IExpressionStats*>& kExprStats,
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

Result compileRollAvgFunc(const Ref<const ExpressionParser::Parse> kParse,
                          const Vec<const StateVector*> kSvs,
                          const IExprNode<F64>*& kNode,
                          Vec<const IExpression*>& kExprNodes,
                          Vec<IExpressionStats*>& kExprStats,
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
        SF_ASSERT(false);
    }

    // Compile first argument expression; the expression being averaged.
    const IExprNode<F64>* arg1Node = nullptr;
    Result res = compileImpl(argNodes[0]->right,
                             kSvs,
                             arg1Node,
                             kExprNodes,
                             kExprStats,
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
                                      ElementType::UINT32,
                                      arg2Asm,
                                      kErr);
    if (res != SUCCESS)
    {
        return res;
    }
    // const U32 histSize =
        // static_cast<const IExprNode<U32>*>(arg2Asm->root())->evaluate();

    // Create expression stats for computing rolling average.
    ExpressionStats<F64, 10>* const exprStats =
        new ExpressionStats<F64, 10>(*arg1Node);
    kExprStats.push_back(exprStats);

    // Create node which returns the rolling average computed by the stats.
    kNode = new RollAvgNode(*exprStats);
    kExprNodes.push_back(kNode);

    return SUCCESS;
}

Result compileFunction(const Ref<const ExpressionParser::Parse> kParse,
                       const Vec<const StateVector*> kSvs,
                       const IExprNode<F64>*& kNode,
                       Vec<const IExpression*>& kExprNodes,
                       Vec<IExpressionStats*>& kExprStats,
                       ErrorInfo* const kErr)
{
    SF_ASSERT(kParse != nullptr);

    if (kParse->data.str == "ROLL_AVG")
    {
        return compileRollAvgFunc(kParse,
                                  kSvs,
                                  kNode,
                                  kExprNodes,
                                  kExprStats,
                                  kErr);
    }

    // If we got this far, the function is not recognized.
    SF_ASSERT(false);
    return -1;
}

Result compileOperator(const Ref<const ExpressionParser::Parse> kParse,
                       const Vec<const StateVector*> kSvs,
                       const IExprNode<F64>*& kNode,
                       Vec<const IExpression*>& kExprNodes,
                       Vec<IExpressionStats*>& kExprStats,
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
    const IExprNode<F64>* nodeRight = nullptr;
    Result res = compileImpl(kParse->right,
                             kSvs,
                             nodeRight,
                             kExprNodes,
                             kExprStats,
                             kErr);
    if (res != SUCCESS)
    {
        return res;
    }
    SF_ASSERT(nodeRight != nullptr);

    // If a binary operator, compile left subtree.
    const IExprNode<F64>* nodeLeft = nullptr;
    if (!opInfo.unary)
    {
        SF_ASSERT(kParse->left != nullptr);
        res = compileImpl(kParse->left,
                          kSvs,
                          nodeLeft,
                          kExprNodes,
                          kExprStats,
                          kErr);
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
    kExprNodes.push_back(kNode);

    return SUCCESS;
}

Result compileImpl(const Ref<const ExpressionParser::Parse> kParse,
                   const Vec<const StateVector*> kSvs,
                   const IExprNode<F64>*& kNode,
                   Vec<const IExpression*>& kExprNodes,
                   Vec<IExpressionStats*>& kExprStats,
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
        return compileFunction(kParse,
                               kSvs,
                               kNode,
                               kExprNodes,
                               kExprStats,
                               kErr);
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

        kExprNodes.push_back(kNode);
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
                const IExprNode<I8>* const nodeElem = new ElementExprNode<I8>(
                    *static_cast<const Element<I8>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I8>(safeCast<F64, I8>,
                                                     *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT16:
            {
                const IExprNode<I16>* const nodeElem = new ElementExprNode<I16>(
                    *static_cast<const Element<I16>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I16>(safeCast<F64, I16>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT32:
            {
                const IExprNode<I32>* const nodeElem = new ElementExprNode<I32>(
                    *static_cast<const Element<I32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I32>(safeCast<F64, I32>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT64:
            {
                const IExprNode<I64>* const nodeElem = new ElementExprNode<I64>(
                    *static_cast<const Element<I64>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I64>(safeCast<F64, I64>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT8:
            {
                const IExprNode<U8>* const nodeElem = new ElementExprNode<U8>(
                    *static_cast<const Element<U8>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U8>(safeCast<F64, U8>,
                                                     *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT16:
            {
                const IExprNode<U16>* const nodeElem = new ElementExprNode<U16>(
                    *static_cast<const Element<U16>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U16>(safeCast<F64, U16>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT32:
            {
                const IExprNode<U32>* const nodeElem = new ElementExprNode<U32>(
                    *static_cast<const Element<U32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U32>(safeCast<F64, U32>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT64:
            {
                const IExprNode<U64>* const nodeElem = new ElementExprNode<U64>(
                    *static_cast<const Element<U64>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U64>(safeCast<F64, U64>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT32:
            {
                const IExprNode<F32>* const nodeElem = new ElementExprNode<F32>(
                    *static_cast<const Element<F32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, F32>(safeCast<F64, F32>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT64:
                kNode = new ElementExprNode<F64>(
                    *static_cast<const Element<F64>*>(elemObj));
                break;

            case ElementType::BOOL:
            {
                const IExprNode<bool>* const nodeElem =
                    new ElementExprNode<bool>(
                        *static_cast<const Element<bool>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, bool>(safeCast<F64, bool>,
                                                       *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }
        }

        // Assert that element type was recognized.
        SF_ASSERT(kNode != nullptr);

        kExprNodes.push_back(kNode);
    }
    else
    {
        // Compile operator expression node.
        const Result res = compileOperator(kParse,
                                           kSvs,
                                           kNode,
                                           kExprNodes,
                                           kExprStats,
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

ExpressionCompiler::Assembly::Assembly(const IExpression* const kRoot,
                                       const Vec<const IExpression*> kNodes,
                                       const Vec<IExpressionStats*> kStats) :
    mRoot(kRoot), mNodes(kNodes), mStats(kStats)
{
}

const Vec<IExpressionStats*>& ExpressionCompiler::Assembly::stats()
    const
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

    // Delete expression stats.
    for (const IExpressionStats* const stats : mStats)
    {
        delete stats;
    }
}

const IExpression* ExpressionCompiler::Assembly::root() const
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

    // Allocated expression nodes will be collected in this vector.
    Vec<const IExpression*> exprNodes;

    // Allocated expression stats will be collected in this vector.
    Vec<IExpressionStats*> exprStats;

    // Compile expression starting at root.
    const IExprNode<F64>* root = nullptr;
    const Result res = compileImpl(kParse,
                                   kSvs,
                                   root,
                                   exprNodes,
                                   exprStats,
                                   kErr);
    if (res != SUCCESS)
    {
        // Delete all allocations since aborting compilation.
        for (const IExpression* const node : exprNodes)
        {
            delete node;
        }

        for (const IExpressionStats* const stats : exprStats)
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

    exprNodes.push_back(newRoot);

    // Return compiled expression assembly.
    kAsm.reset(new ExpressionCompiler::Assembly(newRoot, exprNodes, exprStats));

    return SUCCESS;
}
