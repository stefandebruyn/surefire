#include "sf/config/LanguageConstants.hpp"

const String LangConst::funcNameRollAvg = "ROLL_AVG";

const String LangConst::funcNameRollMedian = "ROLL_MEDIAN";

const String LangConst::funcNameRollMin = "ROLL_MIN";

const String LangConst::funcNameRollMax = "ROLL_MAX";

const String LangConst::funcNameRollRange = "ROLL_RANGE";

const U32 LangConst::rollWindowMaxSize = 100000;

const String LangConst::elemNameStateTime = "T";

const String LangConst::elemNameGlobalTime = "G";

const String LangConst::elemNameState = "S";

const String LangConst::labelEntry = ".ENTRY";

const String LangConst::labelStep = ".STEP";

const String LangConst::labelExit = ".EXIT";

const String LangConst::annotationAssert = "@ASSERT";

const std::regex LangConst::annotationAliasRegex(
    "@ALIAS=([a-zA-Z][a-zA-Z0-9_]*)");

const String LangConst::annotationReadOnly = "@READ_ONLY";

const String LangConst::annotationStop = "@STOP";

const String LangConst::sectionStateVector = "[STATE_VECTOR]";

const String LangConst::sectionLocal = "[LOCAL]";

const String LangConst::keywordIf = "IF";

const String LangConst::keywordElse = "ELSE";

const String LangConst::keywordTransition = "->";

const String LangConst::constantTrue = "TRUE";

const String LangConst::constantFalse = "FALSE";

const TypeInfo TypeInfo::i8 =
    {ElementType::INT8, "I8", 1, true, false, true};

const TypeInfo TypeInfo::i16 =
    {ElementType::INT16, "I16", 2, true, false, true};

const TypeInfo TypeInfo::i32 =
    {ElementType::INT32, "I32", 4, true, false, true};

const TypeInfo TypeInfo::i64 =
    {ElementType::INT64, "I64", 8, true, false, true};

const TypeInfo TypeInfo::u8 =
    {ElementType::UINT8, "U8", 1, true, false, false};

const TypeInfo TypeInfo::u16 =
    {ElementType::UINT16, "U16", 2, true, false, false};

const TypeInfo TypeInfo::u32 =
    {ElementType::UINT32, "U32", 4, true, false, false};

const TypeInfo TypeInfo::u64 =
    {ElementType::UINT64, "U64", 8, true, false, false};

const TypeInfo TypeInfo::f32 =
    {ElementType::FLOAT32, "F32", 4, true, true, true};

const TypeInfo TypeInfo::f64 =
    {ElementType::FLOAT64, "F64", 8, true, true, true};

const TypeInfo TypeInfo::boolean =
    {ElementType::BOOL, "bool", 1, false, false, false};

const Map<String, TypeInfo> TypeInfo::fromName =
{
    {i8.name, i8},
    {i16.name, i16},
    {i32.name, i32},
    {i64.name, i64},
    {u8.name, u8},
    {u16.name, u16},
    {u32.name, u32},
    {u64.name, u64},
    {f32.name, f32},
    {f64.name, f64},
    {boolean.name, boolean},
    {"BOOL", boolean} // Alias for `bool`
};

const Map<ElementType, TypeInfo, EnumHash> TypeInfo::fromEnum =
{
    {i8.enumVal, i8},
    {i16.enumVal, i16},
    {i32.enumVal, i32},
    {i64.enumVal, i64},
    {u8.enumVal, u8},
    {u16.enumVal, u16},
    {u32.enumVal, u32},
    {u64.enumVal, u64},
    {f32.enumVal, f32},
    {f64.enumVal, f64},
    {boolean.enumVal, boolean}
};

const OpInfo OpInfo::lnot =
    {OpInfo::Type::NOT, "NOT", 11, true, false, false, true};

const OpInfo OpInfo::mult =
    {OpInfo::Type::MULT, "*", 10, false, true, true, false};

const OpInfo OpInfo::divide =
    {OpInfo::Type::DIV, "/", 10, false, true, true, false};

const OpInfo OpInfo::add =
    {OpInfo::Type::ADD, "+", 9, false, true, true, false};

const OpInfo OpInfo::sub =
    {OpInfo::Type::SUB, "-", 9, false, true, true, false};

const OpInfo OpInfo::lt =
    {OpInfo::Type::LT, "<", 8, false, false, true, false};

const OpInfo OpInfo::lte =
    {OpInfo::Type::LTE, "<=", 8, false, false, true, false};

const OpInfo OpInfo::gt =
    {OpInfo::Type::GT, ">", 8, false, false, true, false};

const OpInfo OpInfo::gte =
    {OpInfo::Type::GTE, ">=", 8, false, false, true, false};

const OpInfo OpInfo::eq =
    {OpInfo::Type::EQ, "==", 7, false, false, true, true};

const OpInfo OpInfo::neq =
    {OpInfo::Type::NEQ, "!=", 7, false, false, true, true};

const OpInfo OpInfo::land =
    {OpInfo::Type::AND, "AND", 6, false, false, false, true};

const OpInfo OpInfo::lor =
    {OpInfo::Type::OR, "OR", 5, false, false, false, true};

const Map<String, OpInfo> OpInfo::fromStr =
{
    {lnot.str, lnot},
    {"!", lnot}, // Alias for `NOT`
    {mult.str, mult},
    {divide.str, divide},
    {add.str, add},
    {sub.str, sub},
    {lt.str, lt},
    {lte.str, lte},
    {gt.str, gt},
    {gte.str, gte},
    {eq.str, eq},
    {neq.str, neq},
    {land.str, land},
    {lor.str, lor}
};

const Set<String> OpInfo::relOps = {lt.str, lte.str, gt.str, gte.str};
