////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/core/Expression.hpp
/// @brief Objects for representing expression trees.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_EXPRESSION_HPP
#define SF_EXPRESSION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Result.hpp"

///
/// @brief Abstract interface for an expression tree node.
///
/// The IExpression inheritance hierarchy and interfaces of its child classes
/// are designed to implement mixed-type expression trees of constants,
/// StateVector elements, binary and unary operators, and function calls. Class
/// interfaces are also designed to allow RTTI of expression tree nodes so that
/// an autocoder may reconstruct the original source code for the tree.
///
class IExpression
{
public:

    ///
    /// @brief Expression node types.
    ///
    /// Each value in this enum corresponds to a concrete child class of
    /// IExpression. The implementer of a new child class is obligated to add a
    /// new value to this enum and return that value in the
    /// IExpression::nodeType() implementation for their child class.
    ///
    enum NodeType : U8
    {
        CONST = 0,
        ELEMENT = 1,
        BIN_OP = 2,
        UNARY_OP = 3,
        ROLL_AVG = 4,
        ROLL_MEDIAN = 5,
        ROLL_MIN = 6,
        ROLL_MAX = 7,
        ROLL_RANGE = 8
    };

    ///
    /// @brief Destructor.
    ///
    virtual ~IExpression() = default;

    ///
    /// @brief Gets the type which the expression evaluates to.
    ///
    /// @return Expression evaluation type.
    ///
    virtual ElementType type() const = 0;

    ///
    /// @brief Gets the expression node type.
    ///
    /// @see IExpression::NodeType
    ///
    /// @return Expression node type.
    ///
    virtual IExpression::NodeType nodeType() const = 0;
};

///
/// @brief Abstract interface for an expression tree node which evaluates to
/// a particular type.
///
/// @tparam T  Node evaluation type.
///
template<typename T>
class IExprNode : virtual public IExpression
{
public:

    ///
    /// @brief Constructor.
    ///
    IExprNode() = default;

    ///
    /// @brief Destructor.
    ///
    virtual ~IExprNode() = default;

    ///
    /// @brief Evaluates the expression tree rooted at this node.
    ///
    /// @return  Expression value.
    ///
    virtual T evaluate() = 0;

    ///
    /// @see IExpression::type()
    ///
    ElementType type() const final override;

    IExprNode(IExprNode<T>&) = delete;
    IExprNode(IExprNode<T>&&) = delete;
    IExprNode<T>& operator=(IExprNode<T>&) = delete;
    IExprNode<T>& operator=(IExprNode<T>&&) = delete;
};

///
/// @brief Expression tree leaf node that evaluates to a constant.
///
/// @tparam T  Node evaluation type.
///
template<typename T>
class ConstExprNode final : public IExprNode<T>
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kVal  Node value.
    ///
    ConstExprNode(const T kVal) : mVal(kVal)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mVal;
    }

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override
    {
        return IExpression::CONST;
    }

    ///
    /// @brief Gets the node value.
    ///
    /// @return Node value.
    ///
    T val() const
    {
        return mVal;
    }

private:

    ///
    /// @brief Node value.
    ///
    const T mVal;
};

///
/// @brief Abstract interface for the ElementExprNode template.
///
/// @remark This interface helps reduce boilerplate in an autocoder by
/// allowing access to ElementExprNode members without downcasting to a specific
/// instantiation of the ElementExprNode template.
///
class IElementExprNode : virtual public IExpression
{
public:

    ///
    /// @brief Gets the IElement underlying the node.
    ///
    /// @return Node element.
    ///
    virtual const IElement& elem() const = 0;
};

///
/// @brief Expression tree leaf node that evaluates to a state vector element.
///
/// @tparam T  Element type.
///
template<typename T>
class ElementExprNode final : public IExprNode<T>, public IElementExprNode
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kElem  Element which node evaluates to. The element must live
    ///                   at least as long as the expression node.
    ///
    ElementExprNode(const Element<T>& kElem) : mElem(kElem)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mElem.read();
    }

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override
    {
        return IExpression::ELEMENT;
    }

    ///
    /// @see IElementExprNode::elem()
    ///
    const IElement& elem() const final override
    {
        return static_cast<const IElement&>(mElem);
    }

private:

    ///
    /// @brief Element which node evaluates to.
    ///
    const Element<T>& mElem;
};

///
/// @brief Abstract interface for the BinOpExprNode and UnaryOpExprNode
/// templates.
///
/// @remark This interface helps reduce boilerplate in an autocoder by
/// allowing access to BinOpExprNode/UnaryOpExprNode members without downcasting
/// to a specific instantiation of these templates.
///
class IOpExprNode : virtual public IExpression
{
public:

    ///
    /// @brief Gets the address of the node operator function.
    ///
    /// @remark The address is returned as a void* since the signature of the
    /// operator function may vary with node; also, the signature doesn't
    /// matter, as an autocoder can determine the function identity by comparing
    /// to the addresses of functions in the ExprOpFuncs namespace.
    ///
    /// @return Operator function address.
    ///
    virtual const void* op() const = 0;

    ///
    /// @brief Gets a pointer to the LHS expression root node, or null if none.
    ///
    /// @return LHS expression pointer.
    ///
    virtual const IExpression* lhs() const = 0;

    ///
    /// @brief Gets a pointer to the RHS expression root node, or null if none.
    ///
    /// @return RHS expression pointer.
    ///
    virtual const IExpression* rhs() const = 0;
};

///
/// @brief Expression tree node that performs a binary operation on two
/// root nodes.
///
/// @tparam T         Evaluation type.
/// @tparam TOperand  Operand type.
///
template<typename T, typename TOperand = T>
class BinOpExprNode final : public IExprNode<T>, public IOpExprNode
{
public:

    ///
    /// @brief Signature for a binary operation.
    ///
    /// @param[in] kLhs  LHS operand.
    /// @param[in] kRhs  RHS operand.
    ///
    /// @return Operation result.
    ///
    typedef T (*Operator)(const TOperand kLhs, const TOperand kRhs);

    ///
    /// @brief Constructor.
    ///
    /// @remark Only functions in the ExprOpFuncs namespace should be passed for
    /// kOp. This allows an autocoder to determine the function which was used
    /// to construct the BinOpExprNode by simply comparing function pointers.
    ///
    /// @param[in] kOp   Node operation.
    /// @param[in] kLhs  LHS root node.
    /// @param[in] kRhs  RHS root node.
    ///
    BinOpExprNode(const Operator kOp,
                  IExprNode<TOperand>& kLhs,
                  IExprNode<TOperand>& kRhs) :
        mOp(kOp), mLhs(kLhs), mRhs(kRhs)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mOp(mLhs.evaluate(), mRhs.evaluate());
    }

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override
    {
        return IExpression::BIN_OP;
    }

    ///
    /// @see IOpExprNode::op()
    ///
    const void* op() const final override
    {
        return reinterpret_cast<const void*>(mOp);
    }

    ///
    /// @see IOpExprNode::lhs()
    ///
    const IExpression* lhs() const final override
    {
        return &mLhs;
    }

    ///
    /// @see IOpExprNode::rhs()
    ///
    const IExpression* rhs() const final override
    {
        return &mRhs;
    }

private:

    ///
    /// @brief Node operation.
    ///
    const Operator mOp;

    ///
    /// @brief LHS root node.
    ///
    IExprNode<TOperand>& mLhs;

    ///
    /// @brief RHS root node.
    ///
    IExprNode<TOperand>& mRhs;
};

///
/// @brief Expression tree node that performs a unary operation on a root node.
///
/// @note The implementation refers to the operand as the RHS since most unary
/// operators appear to the left of their operand, but this is not a
/// requirement.
///
/// @tparam T         Evaluation type.
/// @tparam TOperand  Operand type.
///
template<typename T, typename TOperand = T>
class UnaryOpExprNode final : public IExprNode<T>, public IOpExprNode
{
public:

    ///
    /// @brief Signature for a unary operation.
    ///
    /// @param[in] kRhs  Operand.
    ///
    /// @return Operation result.
    ///
    typedef T (*Operator)(const TOperand kRhs);

    ///
    /// @brief Constructor.
    ///
    /// @remark Only functions in the ExprOpFuncs namespace should be passed for
    /// kOp. This allows an autocoder to determine the function which was used
    /// to construct the UnaryOpExprNode by simply comparing function pointers.
    ///
    /// @param[in] kOp     Node operation.
    /// @param[in] kRight  Operand root node.
    ///
    UnaryOpExprNode(const Operator kOp, IExprNode<TOperand>& kRhs) :
        mOp(kOp), mRhs(kRhs)
    {
    }

    ///
    /// @see IExprNode<T>::evaluate()
    ///
    T evaluate() final override
    {
        return mOp(mRhs.evaluate());
    }

    ///
    /// @see IExpression::nodeType()
    ///
    IExpression::NodeType nodeType() const final override
    {
        return IExpression::UNARY_OP;
    }

    ///
    /// @see IOpExprNode::op()
    ///
    const void* op() const final override
    {
        return reinterpret_cast<const void*>(mOp);
    }

    ///
    /// @see IOpExprNode::lhs()
    ///
    const IExpression* lhs() const final override
    {
        return nullptr;
    }

    ///
    /// @see IOpExprNode::rhs()
    ///
    const IExpression* rhs() const final override
    {
        return &mRhs;
    }

private:

    ///
    /// @brief Node operation.
    ///
    const Operator mOp;

    ///
    /// @brief Operand root node.
    ///
    IExprNode<TOperand>& mRhs;
};

///
/// @brief Namespace of binary and unary operator functions.
///
namespace ExprOpFuncs
{
    ///
    /// @brief Addition operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T add(const T kLhs, const T kRhs)
    {
        return (kLhs + kRhs);
    }

    ///
    /// @brief Subtraction operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T sub(const T kLhs, const T kRhs)
    {
        return (kLhs - kRhs);
    }

    ///
    /// @brief Multiplication operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T mult(const T kLhs, const T kRhs)
    {
        return (kLhs * kRhs);
    }

    ///
    /// @brief Division operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T div(const T kLhs, const T kRhs)
    {
        return (kLhs / kRhs);
    }

    ///
    /// @brief Less-than operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T lt(const T kLhs, const T kRhs)
    {
        return (kLhs < kRhs);
    }

    ///
    /// @brief Less-than-or-equals operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T lte(const T kLhs, const T kRhs)
    {
        return (kLhs <= kRhs);
    }

    ///
    /// @brief Greater-than operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T gt(const T kLhs, const T kRhs)
    {
        return (kLhs > kRhs);
    }

    ///
    /// @brief Greater-than-or-equals operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T gte(const T kLhs, const T kRhs)
    {
        return (kLhs >= kRhs);
    }

    ///
    /// @brief Equals operator function.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T eq(const T kLhs, const T kRhs)
    {
        return (kLhs == kRhs);
    }

    ///
    /// @brief Not-equals operator function.
    ///
    /// @remark The return type is the same as the operand type to simplify
    /// instantiations of this template by config compilers/autocoders.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T neq(const T kLhs, const T kRhs)
    {
        return (kLhs != kRhs);
    }

    ///
    /// @brief Logical AND operator function.
    ///
    /// @remark The return type is the same as the operand type to simplify
    /// instantiations of this template by config compilers/autocoders.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T land(const T kLhs, const T kRhs)
    {
        return (kLhs && kRhs);
    }

    ///
    /// @brief Logical OR operator function.
    ///
    /// @remark The return type is the same as the operand type to simplify
    /// instantiations of this template by config compilers/autocoders.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T lor(const T kLhs, const T kRhs)
    {
        return (kLhs || kRhs);
    }

    ///
    /// @brief Logical NOT operator function.
    ///
    /// @remark The return type is the same as the operand type to simplify
    /// instantiations of this template by config compilers/autocoders.
    ///
    /// @tparam T  Operand and return type.
    ///
    /// @param[in] kLhs  LHS value.
    /// @param[in] kRhs  RHS value.
    ///
    /// @return Operation result.
    ///
    template<typename T>
    T lnot(const T kRhs)
    {
        return !kRhs;
    }

    ///
    /// @brief A "safe" cast operation that saturates at numeric limits when
    /// precision loss prevents a precise cast and converts NaNs to 0 or false.
    /// Casting an integral type to a floating type when the integer cannot be
    /// accurately represented as a float has the same effect as a static cast
    /// (i.e., the integer is approximated as a float).
    ///
    /// @remark This is currently only defined to/from F64 since these are the
    /// only casts required by config library compilers.
    ///
    /// @tparam T         Cast (destination) type.
    /// @tparam TOperand  Operand (source) type.
    ///
    /// @param[in] kRhs  Value to cast.
    ///
    /// @return Cast value.
    ///
    template<typename T, typename TOperand>
    T safeCast(const TOperand kRhs);
}

///
/// @brief Namespace of templates for getting numeric type limits, similar to
/// the <limits> stdlib header.
///
namespace Limits
{
    ///
    /// @brief Gets the minimum value representable by a numeric type. For
    /// floating types, this is negative infinity.
    ///
    /// @tparam T  Type.
    ///
    /// @return Minimum value representable by type.
    ///
    template<typename T>
    T min();

    ///
    /// @brief Gets the maximum value representable by a numeric type. For
    /// floating types, this is positive infinity.
    ///
    /// @tparam T  Type.
    ///
    /// @return Maximum value representable by type.
    ///
    template<typename T>
    T max();
}

#endif
