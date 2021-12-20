#ifndef SFA_EXPRESSION_NODE_HPP
#define SFA_EXPRESSION_NODE_HPP

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
    OP_OR
};

template <typename T>
class ExpressionTree final
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
                             const ExpressionTree<T>* kLeft,
                             const ExpressionTree<T>* kRight) :
        ExpressionTree(0, nullptr, kOp, kLeft, kRight)
    {
    }

    ExpressionTree(const ExpressionTree<T>&) = delete;
    ExpressionTree(ExpressionTree<T>&&) = delete;
    ExpressionTree<T>& operator=(const ExpressionTree<T>&) = delete;
    ExpressionTree<T>& operator=(ExpressionTree<T>&&) = delete;

    Result evaluate(T& kAns) const
    {
        if (mOp != OP_NONE)
        {
            // Operation node.

            // Check that left and right subtrees are non-null.
            if ((mLeft == nullptr) || (mRight == nullptr))
            {
                return E_NULLPTR;
            }

            // Evaluate left subtree.
            T leftValue = 0;
            Result res = mLeft->evaluate(leftValue);
            if (res != SUCCESS)
            {
                return res;
            }

            // Evaluate right subtree.
            T rightValue = 0;
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

private:

    const T mValue;

    const Element<T>* mElem;

    const ExpressionTreeOperator mOp;

    const ExpressionTree<T>* mLeft;

    const ExpressionTree<T>* mRight;

    constexpr ExpressionTree(const T kValue,
                             const Element<T>* kElem,
                             const ExpressionTreeOperator kOp,
                             const ExpressionTree<T>* kLeft,
                             const ExpressionTree<T>* kRight) :
        mValue(kValue), mElem(kElem), mOp(kOp), mLeft(kLeft), mRight(kRight)
    {
    }
};

#endif
