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
/// @file  sf/config/LanguageConstants.hpp
/// @brief Config language constants, especially for the state machine DSL.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_LANGUAGE_CONSTANTS_HPP
#define SF_LANGUAGE_CONSTANTS_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"

///
/// @brief Config language constants.
///
namespace LangConst
{
    ///
    /// @brief Rolling average function identifier.
    ///
    extern const String funcRollAvg;

    ///
    /// @brief Rolling median function identifier.
    ///
    extern const String funcRollMedian;

    ///
    /// @brief Rolling min function identifier.
    ///
    extern const String funcRollMin;

    ///
    /// @brief Rolling max function identifier.
    ///
    extern const String funcRollMax;

    ///
    /// @brief Rolling range function identifier.
    ///
    extern const String funcRollRange;

    ///
    /// @brief Maximum legal window size for a stats function.
    ///
    extern const U32 rollWindowMaxSize;

    ///
    /// @brief State time element name.
    ///
    extern const String elemStateTime;

    ///
    /// @brief Global time element name.
    ///
    extern const String elemGlobalTime;

    ///
    /// @brief State element name.
    ///
    extern const String elemState;

    ///
    /// @brief Delta T option name.
    ///
    extern const String optDeltaT;

    ///
    /// @brief Initial state option name.
    ///
    extern const String optInitState;

    ///
    /// @brief Lock option name.
    ///
    extern const String optLock;

    ///
    /// @brief State entry label.
    ///
    extern const String labelEntry;

    ///
    /// @brief State step label.
    ///
    extern const String labelStep;

    ///
    /// @brief State exit label.
    ///
    extern const String labelExit;

    ///
    /// @brief Assert annotation.
    ///
    extern const String annotationAssert;

    ///
    /// @brief Alias annotation.
    ///
    extern const String annotationAlias;

    ///
    /// @brief Read-only annotation.
    ///
    extern const String annotationReadOnly;

    ///
    /// @brief Stop annotation.
    ///
    extern const String annotationStop;

    ///
    /// @brief State vector section name.
    ///
    extern const String sectionStateVector;

    ///
    /// @brief Local variables section name.
    ///
    extern const String sectionLocal;

    ///
    /// @brief State script all states section name.
    ///
    extern const String sectionAllStates;

    ///
    /// @brief State script options section name.
    ///
    extern const String sectionOptions;

    ///
    /// @brief If keyword.
    ///
    extern const String keywordIf;

    ///
    /// @brief Else keyword.
    ///
    extern const String keywordElse;

    ///
    /// @brief State transition keyword.
    ///
    extern const String keywordTransition;

    ///
    /// @brief Boolean true constant.
    ///
    extern const String constantTrue;

    ///
    /// @brief Boolean false constant.
    ///
    extern const String constantFalse;
}

///
/// @brief Info about a variable/element type.
///
struct TypeInfo final
{
    static const TypeInfo i8;      /// I8 type info.
    static const TypeInfo i16;     /// I16 type info.
    static const TypeInfo i32;     /// I32 type info.
    static const TypeInfo i64;     /// I64 type info.
    static const TypeInfo u8;      /// U8 type info.
    static const TypeInfo u16;     /// U16 type info.
    static const TypeInfo u32;     /// U32 type info.
    static const TypeInfo u64;     /// U64 type info.
    static const TypeInfo f32;     /// F32 type info.
    static const TypeInfo f64;     ///< F64 type info.
    static const TypeInfo boolean; ///< Bool type info.

    ///
    /// @brief Map of type identifiers to info.
    ///
    static const Map<String, TypeInfo> fromName;

    ///
    /// @brief Map of type enums to info.
    ///
    static const Map<ElementType, TypeInfo> fromEnum;

    ElementType enumVal; ///< Enum value.
    String name;         ///< Identifier.
    U32 sizeBytes;       ///< Size in bytes.
    bool arithmetic;     ///< If arithmetic.
    bool fp;             ///< If floating.
    bool sign;           ///< If signed.
};

///
/// @brief Info about an operator.
///
struct OpInfo final
{
    ///
    /// @brief Operator types.
    ///
    enum Type : U8
    {
        NOT = 0,
        MULT = 1,
        DIV = 2,
        ADD = 3,
        SUB = 4,
        LT = 5,
        LTE = 6,
        GT = 7,
        GTE = 8,
        EQ = 9,
        NEQ = 10,
        AND = 11,
        OR = 12
    };
    
    static const OpInfo lnot;   ///< Logical not operator info.
    static const OpInfo mult;   ///< Multiplication operator info.
    static const OpInfo divide; ///< Division operator info.
    static const OpInfo add;    ///< Addition operator info.
    static const OpInfo sub;    ///< Subtraction operator info.
    static const OpInfo lt;     ///< Less than operator info.
    static const OpInfo lte;    ///< Less than or equal operator info.
    static const OpInfo gt;     ///< Greater than operator info.
    static const OpInfo gte;    ///< Greater than or equal operator info.
    static const OpInfo eq;     ///< Equal operator info.
    static const OpInfo neq;    ///< Not equal operator info.
    static const OpInfo land;   ///< Logical and operator info.
    static const OpInfo lor;    ///< Logical or operator info.

    ///
    /// @brief Map of operator strings to info.
    ///
    static const Map<String, OpInfo> fromStr;

    ///
    /// @brief Set of relational operators.
    ///
    static const Set<String> relOps;

    Type enumVal;            ///< Enum value.
    String str;              ///< String in config languages.
    String cpp;              ///< String in C++.
    U32 precedence;          ///< Precedence.
    bool unary;              ///< If unary
    bool arithmetic;         ///< If arithmetic.
    bool arithmeticOperands; ///< If can have arithmetic operands.
    bool logicalOperands;    ///< If can have logical operands.
};

#endif
