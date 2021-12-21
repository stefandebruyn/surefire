#ifndef SFA_EXPRESSION_TREE_HPP
#define SFA_EXPRESSION_TREE_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/Result.hpp"

enum ExpressionTreeOperator : U32
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

template <typename T>
class IExpressionTree
{
public:

    virtual ~IExpressionTree() = default;

    virtual Result evaluate(T& kAns) const = 0;
};

template <typename T, typename TLeft = T, typename TRight = TLeft>
class ExpressionTree final : public IExpressionTree<T>
{
public:

    constexpr ExpressionTree(const T kValue) :
        ExpressionTree(kValue, nullptr, OP_NONE, nullptr, nullptr)
    {
    }

    constexpr ExpressionTree(const Element<T>* kElem) :
        ExpressionTree(0, kElem, OP_NONE, nullptr, nullptr)
    {
    }

    constexpr ExpressionTree(const ExpressionTreeOperator kOp,
                             const IExpressionTree<TLeft>* kLeft,
                             const IExpressionTree<TRight>* kRight) :
        ExpressionTree(0, nullptr, kOp, kLeft, kRight)
    {
    }

    ExpressionTree(const ExpressionTree<T, TLeft, TRight>&) = delete;
    ExpressionTree(ExpressionTree<T, TLeft, TRight>&&) = delete;
    ExpressionTree<T, TLeft, TRight>& operator=(
        const ExpressionTree<T, TLeft, TRight>&) = delete;
    ExpressionTree<T, TLeft, TRight>& operator=(
        ExpressionTree<T, TLeft, TRight>&&) = delete;

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
                    kAns = (leftValue * rightValue);
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
            if (mElem == nullptr)
            {
                return E_NULLPTR;
            }
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

private:

    const T mValue;

    const Element<T>* mElem;

    const ExpressionTreeOperator mOp;

    const IExpressionTree<TLeft>* mLeft;

    const IExpressionTree<TRight>* mRight;

    constexpr ExpressionTree(const T kValue,
                             const Element<T>* kElem,
                             const ExpressionTreeOperator kOp,
                             const IExpressionTree<TLeft>* kLeft,
                             const IExpressionTree<TRight>* kRight) :
        mValue(kValue), mElem(kElem), mOp(kOp), mLeft(kLeft), mRight(kRight)
    {
    }
};

#endif
