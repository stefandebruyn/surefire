#ifndef SFA_EXPRESSION_NODE_HPP
#define SFA_EXPRESSION_NODE_HPP

#include "sfa/core/Element.hpp"
#include "sfa/core/Result.hpp"

enum ExpressionNodeOperator : U32
{
    OP_NONE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_AND,
    OP_OR,
    OP_LESS_THAN,
    OP_LESS_THAN_EQUALS,
    OP_GREATER_THAN,
    OP_GREATER_THAN_EQUALS,
    OP_EQUALS,
    OP_NOT_EQUALS
};

template<typename T>
class IExpressionNode
{
public:

    virtual ~IExpressionNode() = default;

    virtual Result evaluate(T& kAns) const = 0;
};

template<typename T, typename TLeft = T, typename TRight = TLeft>
class ExpressionNode final : public IExpressionNode<T>
{
public:

    constexpr ExpressionNode(const T kValue) :
        ExpressionNode(kValue, nullptr, OP_NONE, nullptr, nullptr)
    {
    }

    constexpr ExpressionNode(const Element<T>& kElem) :
        ExpressionNode(0, &kElem, OP_NONE, nullptr, nullptr)
    {
    }

    constexpr ExpressionNode(const ExpressionNodeOperator kOp,
                             const IExpressionNode<TLeft>* const kLeft,
                             const IExpressionNode<TRight>* const kRight) :
        ExpressionNode(0, nullptr, kOp, kLeft, kRight)
    {
    }

    Result evaluate(T& kAns) const final override
    {
        if (mOp != OP_NONE)
        {
            // Operator node.

            // Check that left and right subtrees are non-null.
            if ((mLeft == nullptr) || (mRight == nullptr))
            {
                return E_NULLPTR;
            }

            // Evaluate left subtree.
            TLeft leftValue = 0;
            Result res = mLeft->evaluate(leftValue);
            if (res != SUCCESS)
            {
                return res;
            }

            // Evaluate right subtree.
            TRight rightValue = 0;
            res = mRight->evaluate(rightValue);
            if (res != SUCCESS)
            {
                return res;
            }

            // Apply operation to operands.
            switch (mOp)
            {
                case OP_ADD:
                    kAns = (leftValue + rightValue);
                    break;

                case OP_SUBTRACT:
                    kAns = (leftValue - rightValue);
                    break;

                case OP_MULTIPLY:
                    // FIXME: Temporary nop to avoid compiler warning, pending
                    // state machine rework.
                    // kAns = (leftValue * rightValue);
                    break;

                case OP_DIVIDE:
                    kAns = (leftValue / rightValue);
                    break;

                case OP_AND:
                    kAns = (leftValue && rightValue);
                    break;

                case OP_OR:
                    kAns = (leftValue || rightValue);
                    break;

                case OP_LESS_THAN:
                    kAns = (leftValue < rightValue);
                    break;

                case OP_LESS_THAN_EQUALS:
                    kAns = (leftValue <= rightValue);
                    break;

                case OP_GREATER_THAN:
                    kAns = (leftValue > rightValue);
                    break;

                case OP_GREATER_THAN_EQUALS:
                    kAns = (leftValue >= rightValue);
                    break;

                case OP_EQUALS:
                    kAns = (leftValue == rightValue);
                    break;

                case OP_NOT_EQUALS:
                    kAns = (leftValue != rightValue);
                    break;

                default:
                    return E_ENUM;
            }
        }
        else if (mElem != nullptr)
        {
            // State vector element node.
            kAns = mElem->read();
        }
        else
        {
            // Constant node.
            kAns = mValue;
        }

        return SUCCESS;
    }

    ExpressionNode(const ExpressionNode<T, TLeft, TRight>&) = delete;
    ExpressionNode(ExpressionNode<T, TLeft, TRight>&&) = delete;
    ExpressionNode<T, TLeft, TRight>& operator=(
        const ExpressionNode<T, TLeft, TRight>&) = delete;
    ExpressionNode<T, TLeft, TRight>& operator=(
        ExpressionNode<T, TLeft, TRight>&&) = delete;

private:

    const T mValue;

    const Element<T>* const mElem;

    const ExpressionNodeOperator mOp;

    const IExpressionNode<TLeft>* const mLeft;

    const IExpressionNode<TRight>* const mRight;

    constexpr ExpressionNode(const T kValue,
                             const Element<T>* kElem,
                             const ExpressionNodeOperator kOp,
                             const IExpressionNode<TLeft>* kLeft,
                             const IExpressionNode<TRight>* kRight) :
        mValue(kValue), mElem(kElem), mOp(kOp), mLeft(kLeft), mRight(kRight)
    {
    }
};

#endif
