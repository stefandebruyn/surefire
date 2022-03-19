#include <algorithm>

#include "sf/config/ConfigUtil.hpp"

const ElementTypeInfo ElementTypeInfo::i8 =
    {ElementType::INT8, "I8", 1, true, false, true};
const ElementTypeInfo ElementTypeInfo::i16 =
    {ElementType::INT16, "I16", 2, true, false, true};
const ElementTypeInfo ElementTypeInfo::i32 =
    {ElementType::INT32, "I32", 4, true, false, true};
const ElementTypeInfo ElementTypeInfo::i64 =
    {ElementType::INT64, "I64", 8, true, false, true};
const ElementTypeInfo ElementTypeInfo::u8 =
    {ElementType::UINT8, "U8", 1, true, false, false};
const ElementTypeInfo ElementTypeInfo::u16 =
    {ElementType::UINT16, "U16", 2, true, false, false};
const ElementTypeInfo ElementTypeInfo::u32 =
    {ElementType::UINT32, "U32", 4, true, false, false};
const ElementTypeInfo ElementTypeInfo::u64 =
    {ElementType::UINT64, "U64", 8, true, false, false};
const ElementTypeInfo ElementTypeInfo::f32 =
    {ElementType::FLOAT32, "F32", 4, true, true, true};
const ElementTypeInfo ElementTypeInfo::f64 =
    {ElementType::FLOAT64, "F64", 8, true, true, true};
const ElementTypeInfo ElementTypeInfo::boolean =
    {ElementType::BOOL, "bool", 1, false, false, false};

const Map<String, ElementTypeInfo>
    ElementTypeInfo::fromName =
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

const Map<ElementType, ElementTypeInfo, EnumHash>
    ElementTypeInfo::fromEnum =
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

const OperatorInfo OperatorInfo::lnot =
    {OperatorInfo::Type::NOT, "NOT", 11, true, false, false, true};
const OperatorInfo OperatorInfo::mult =
    {OperatorInfo::Type::MULT, "*", 10, false, true, true, false};
const OperatorInfo OperatorInfo::divide =
    {OperatorInfo::Type::DIV, "/", 10, false, true, true, false};
const OperatorInfo OperatorInfo::add =
    {OperatorInfo::Type::ADD, "+", 9, false, true, true, false};
const OperatorInfo OperatorInfo::sub =
    {OperatorInfo::Type::SUB, "-", 9, false, true, true, false};
const OperatorInfo OperatorInfo::lt =
    {OperatorInfo::Type::LT, "<", 8, false, false, true, false};
const OperatorInfo OperatorInfo::lte =
    {OperatorInfo::Type::LTE, "<=", 8, false, false, true, false};
const OperatorInfo OperatorInfo::gt =
    {OperatorInfo::Type::GT, ">", 8, false, false, true, false};
const OperatorInfo OperatorInfo::gte =
    {OperatorInfo::Type::GTE, ">=", 8, false, false, true, false};
const OperatorInfo OperatorInfo::eq =
    {OperatorInfo::Type::EQ, "==", 7, false, false, true, true};
const OperatorInfo OperatorInfo::neq =
    {OperatorInfo::Type::NEQ, "!=", 7, false, false, true, true};
const OperatorInfo OperatorInfo::land =
    {OperatorInfo::Type::AND, "AND", 6, false, false, false, true};
const OperatorInfo OperatorInfo::lor =
    {OperatorInfo::Type::OR, "OR", 5, false, false, false, true};

const Map<String, OperatorInfo> OperatorInfo::fromStr =
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

const Set<String> OperatorInfo::relOps =
    {lt.str, lte.str, gt.str, gte.str};

const Set<String> ConfigUtil::reserved =
{
    "STATE_VECTOR",
    "LOCAL",
    "IF",
    "ELSE",
    "ENTRY",
    "STEP",
    "EXIT",
    "T",
    "G",
    "S"
};

void ConfigUtil::setError(ErrorInfo* const kErr,
                          const Token& kTokErr,
                          const String kText,
                          const String kSubtext)
{
    if (kErr != nullptr)
    {
        kErr->lineNum = kTokErr.lineNum;
        kErr->colNum = kTokErr.colNum;
        kErr->text = kText;
        kErr->subtext = kSubtext;
    }
}

bool ConfigUtil::checkEof(const TokenIterator& kIt,
                          const Token& kTokLast,
                          const String kErrText,
                          ErrorInfo* const kErr)
{
    if (kIt.eof())
    {
        if (kErr != nullptr)
        {
            kErr->lineNum = kTokLast.lineNum;
            kErr->colNum = (kTokLast.colNum + kTokLast.str.size());
            kErr->text = kErrText;
            kErr->subtext = "unexpected end of file";
        }
        return true;
    }

    return false;
}
