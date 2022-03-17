#include <cstdlib>
#include <cmath>

#include "sf/config/ConfigUtil.hpp"
#include "sf/config/ExpressionCompiler.hpp"
#include "sf/core/Assert.hpp"

/////////////////////////////////// Private ////////////////////////////////////

namespace
{

const char* const errText = "expression error";

Result tokenToF64(const Token& kTok, F64& kRet, ErrorInfo* const kErr);

Result compileOperator(const std::shared_ptr<ExpressionParser::Parse> kParse,
                       const StateVector& kSv,
                       const IExprNode<F64>*& kNode,
                       std::vector<const IExpression*>& kExprNodes,
                       ErrorInfo* const kErr);

Result compileImpl(const std::shared_ptr<ExpressionParser::Parse> kParse,
                   const StateVector& kSv,
                   const IExprNode<F64>*& kNode,
                   std::vector<const IExpression*>& kExprNodes,
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

Result compileOperator(const std::shared_ptr<ExpressionParser::Parse> kParse,
                       const StateVector& kSv,
                       const IExprNode<F64>*& kNode,
                       std::vector<const IExpression*>& kExprNodes,
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
                             kSv,
                             nodeRight,
                             kExprNodes,
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
                          kSv,
                          nodeLeft,
                          kExprNodes,
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
            kNode = new UnaryOpExprNode<F64>(bang<F64>, *nodeRight);
            break;

        case OperatorInfo::Type::MULT:
            kNode = new BinOpExprNode<F64>(
                multiply<F64>, *nodeLeft, *nodeRight);
            break;

        case OperatorInfo::Type::DIV:
            kNode = new BinOpExprNode<F64>(divide<F64>, *nodeLeft, *nodeRight);
            break;

        case OperatorInfo::Type::ADD:
            kNode = new BinOpExprNode<F64>(add<F64>, *nodeLeft, *nodeRight);
            break;

        case OperatorInfo::Type::SUB:
            kNode = new BinOpExprNode<F64>(
                subtract<F64>, *nodeLeft, *nodeRight);
            break;

        case OperatorInfo::Type::LT:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                lessThan<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::LTE:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                lessThanEquals<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::GT:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                greaterThan<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::GTE:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                greaterThanEquals<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::EQ:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                equals<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::NEQ:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                notEquals<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::AND:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                logicalAnd<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
            break;
        }

        case OperatorInfo::Type::OR:
        {
            const IExprNode<bool>* const nodeCmp = new BinOpExprNode<bool, F64>(
                logicalOr<F64>, *nodeLeft, *nodeRight);
            kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>, *nodeCmp);
            kExprNodes.push_back(nodeCmp);
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

Result compileImpl(const std::shared_ptr<ExpressionParser::Parse> kParse,
                   const StateVector& kSv,
                   const IExprNode<F64>*& kNode,
                   std::vector<const IExpression*>& kExprNodes,
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
        SF_ASSERT(false);
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
        const Result res = kSv.getIElement(kParse->data.str.c_str(), elemObj);
        if (res != SUCCESS)
        {
            // Unknown element.
            ConfigUtil::setError(kErr, kParse->data, errText,
                                 "unknown element");
            return E_EXC_ELEM;
        }

        // Assert that element pointer was populated.
        SF_ASSERT(elemObj != nullptr);

        // Narrow the element pointer to a template instantiation of the
        // element's type.
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                const IExprNode<I8>* const nodeElem = new ElementExprNode<I8>(
                    *static_cast<const Element<I8>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I8>(cast<F64, I8>, *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT16:
            {
                const IExprNode<I16>* const nodeElem = new ElementExprNode<I16>(
                    *static_cast<const Element<I16>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I16>(cast<F64, I16>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT32:
            {
                const IExprNode<I32>* const nodeElem = new ElementExprNode<I32>(
                    *static_cast<const Element<I32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I32>(cast<F64, I32>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::INT64:
            {
                const IExprNode<I64>* const nodeElem = new ElementExprNode<I64>(
                    *static_cast<const Element<I64>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, I64>(cast<F64, I64>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT8:
            {
                const IExprNode<U8>* const nodeElem = new ElementExprNode<U8>(
                    *static_cast<const Element<U8>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U8>(cast<F64, U8>, *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT16:
            {
                const IExprNode<U16>* const nodeElem = new ElementExprNode<U16>(
                    *static_cast<const Element<U16>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U16>(cast<F64, U16>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT32:
            {
                const IExprNode<U32>* const nodeElem = new ElementExprNode<U32>(
                    *static_cast<const Element<U32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U32>(cast<F64, U32>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::UINT64:
            {
                const IExprNode<U64>* const nodeElem = new ElementExprNode<U64>(
                    *static_cast<const Element<U64>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, U64>(cast<F64, U64>,
                                                      *nodeElem);
                kExprNodes.push_back(nodeElem);
                break;
            }

            case ElementType::FLOAT32:
            {
                const IExprNode<F32>* const nodeElem = new ElementExprNode<F32>(
                    *static_cast<const Element<F32>*>(elemObj));
                kNode = new UnaryOpExprNode<F64, F32>(cast<F64, F32>,
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
                kNode = new UnaryOpExprNode<F64, bool>(cast<F64, bool>,
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
                                           kSv,
                                           kNode,
                                           kExprNodes,
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

ExpressionCompiler::Assembly::Assembly(
        const IExpression* const kRoot,
        const std::vector<const IExpression*> kNodes) :
    mRoot(kRoot), mNodes(kNodes)
{
}

ExpressionCompiler::Assembly::~Assembly()
{
    for (const IExpression* const node : mNodes)
    {
        delete node;
    }
}

const IExpression* ExpressionCompiler::Assembly::root() const
{
    return mRoot;
}

Result ExpressionCompiler::compile(
    const std::shared_ptr<ExpressionParser::Parse> kParse,
    const StateVector& kSv,
    const ElementType kEvalType,
    std::shared_ptr<ExpressionCompiler::Assembly>& kAsm,
    ErrorInfo* const kErr)
{
    // Check that expression parse is non-null.
    if (kParse == nullptr)
    {
        return E_EXC_NULL;
    }

    // Collect allocated expression nodes in this vector.
    std::vector<const IExpression*> exprNodes;

    // Compile expression starting at root.
    const IExprNode<F64>* root = nullptr;
    const Result res = compileImpl(kParse, kSv, root, exprNodes, kErr);
    if (res != SUCCESS)
    {
        // Aborting compilation, so delete all allocated nodes.
        for (const IExpression* const node : exprNodes)
        {
            delete node;
        }

        return res;
    }

    const IExpression* asmRoot = root;

    // If expression should evaluate to something other than F64, add a cast.
    if (kEvalType != ElementType::FLOAT64)
    {
        IExpression* newRoot = nullptr;
        switch (kEvalType)
        {
            case ElementType::INT8:
                newRoot = new UnaryOpExprNode<I8, F64>(cast<I8, F64>, *root);
                break;

            case ElementType::INT16:
                newRoot = new UnaryOpExprNode<I16, F64>(cast<I16, F64>, *root);
                break;

            case ElementType::INT32:
                newRoot = new UnaryOpExprNode<I32, F64>(cast<I32, F64>, *root);
                break;

            case ElementType::INT64:
                newRoot = new UnaryOpExprNode<I64, F64>(cast<I64, F64>, *root);
                break;

            case ElementType::UINT8:
                newRoot = new UnaryOpExprNode<U8, F64>(cast<U8, F64>, *root);
                break;

            case ElementType::UINT16:
                newRoot = new UnaryOpExprNode<U16, F64>(cast<U16, F64>, *root);
                break;

            case ElementType::UINT32:
                newRoot = new UnaryOpExprNode<U32, F64>(cast<U32, F64>, *root);
                break;

            case ElementType::UINT64:
                newRoot = new UnaryOpExprNode<U64, F64>(cast<U64, F64>, *root);
                break;

            case ElementType::FLOAT32:
                newRoot = new UnaryOpExprNode<F32, F64>(cast<F32, F64>, *root);
                break;

            case ElementType::BOOL:
                newRoot = new UnaryOpExprNode<bool, F64>(cast<bool, F64>,
                                                         *root);
                break;

            default:
                SF_ASSERT(false);
        }

        asmRoot = newRoot;
    }

    // Return compiled expression assembly.
    kAsm.reset(new ExpressionCompiler::Assembly(asmRoot, exprNodes));

    return SUCCESS;
}
