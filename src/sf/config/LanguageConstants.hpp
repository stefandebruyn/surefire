#ifndef SF_LANGUAGE_CONSTANTS_HPP
#define SF_LANGUAGE_CONSTANTS_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"

namespace LangConst
{
    extern const String funcRollAvg;

    extern const String funcRollMedian;

    extern const String funcRollMin;

    extern const String funcRollMax;

    extern const String funcRollRange;

    extern const U32 rollWindowMaxSize;

    extern const String elemStateTime;

    extern const String elemGlobalTime;

    extern const String elemState;

    extern const String optDeltaT;

    extern const String optInitState;

    extern const String labelEntry;

    extern const String labelStep;

    extern const String labelExit;

    extern const String annotationAssert;

    extern const String annotationAlias;

    extern const String annotationReadOnly;

    extern const String annotationStop;

    extern const String sectionStateVector;

    extern const String sectionLocal;

    extern const String sectionAllStates;

    extern const String sectionConfig;

    extern const String keywordIf;

    extern const String keywordElse;

    extern const String keywordTransition;

    extern const String constantTrue;

    extern const String constantFalse;
}

struct TypeInfo final
{
    static const TypeInfo i8;
    static const TypeInfo i16;
    static const TypeInfo i32;
    static const TypeInfo i64;
    static const TypeInfo u8;
    static const TypeInfo u16;
    static const TypeInfo u32;
    static const TypeInfo u64;
    static const TypeInfo f32;
    static const TypeInfo f64;
    static const TypeInfo boolean;

    static const Map<String, TypeInfo> fromName;

    static const Map<ElementType, TypeInfo> fromEnum;

    ElementType enumVal;
    String name;
    U32 sizeBytes;
    bool arithmetic;
    bool fp;
    bool sign;
};

struct OpInfo final
{
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
    
    static const OpInfo lnot;
    static const OpInfo mult;
    static const OpInfo divide;
    static const OpInfo add;
    static const OpInfo sub;
    static const OpInfo lt;
    static const OpInfo lte;
    static const OpInfo gt;
    static const OpInfo gte;
    static const OpInfo eq;
    static const OpInfo neq;
    static const OpInfo land;
    static const OpInfo lor;

    static const Map<String, OpInfo> fromStr;

    static const Set<String> relOps;

    Type enumVal;
    String str;
    U32 precedence;
    bool unary;
    bool arithmetic;
    bool arithmeticOperands;
    bool logicalOperands;
};

#endif
