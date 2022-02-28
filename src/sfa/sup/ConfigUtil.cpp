#include <algorithm>

#include "sfa/sup/ConfigUtil.hpp"

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

const std::unordered_map<std::string, ElementTypeInfo>
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

const std::unordered_map<ElementType, ElementTypeInfo, EnumHash>
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

const OperatorInfo OperatorInfo::lnot = {OP_NOT, "NOT", 11, true};
const OperatorInfo OperatorInfo::neg = {OP_NEG, "-", 11, true};
const OperatorInfo OperatorInfo::mult = {OP_MULT, "*", 10, false};
const OperatorInfo OperatorInfo::divide = {OP_DIV, "/", 10, false};
const OperatorInfo OperatorInfo::mod = {OP_MOD, "%", 10, false};
const OperatorInfo OperatorInfo::add = {OP_ADD, "+", 9, false};
const OperatorInfo OperatorInfo::sub = {OP_SUB, "-", 9, false};
const OperatorInfo OperatorInfo::lt = {OP_LT, "<", 8, false};
const OperatorInfo OperatorInfo::lte = {OP_LTE, "<=", 8, false};
const OperatorInfo OperatorInfo::gt = {OP_GT, ">", 8, false};
const OperatorInfo OperatorInfo::gte = {OP_GTE, ">=", 8, false};
const OperatorInfo OperatorInfo::eq = {OP_EQ, "==", 7, false};
const OperatorInfo OperatorInfo::neq = {OP_NEQ, "!=", 7, false};
const OperatorInfo OperatorInfo::land = {OP_AND, "AND", 6, false};
const OperatorInfo OperatorInfo::lor = {OP_OR, "OR", 5, false};

const std::unordered_map<std::string, OperatorInfo> OperatorInfo::fromStr =
{
    {lnot.str, lnot},
    {"!", lnot}, // Alias for `NOT`
    {neg.str, neg},
    {mult.str, mult},
    {divide.str, divide},
    {mod.str, mod},
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

const std::unordered_set<std::string> ConfigUtil::reserved =
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

void ConfigUtil::setError(ConfigErrorInfo* const kConfigErr,
                          const Token& kTokErr,
                          const std::string kText,
                          const std::string kSubtext)
{
    if (kConfigErr != nullptr)
    {
        kConfigErr->lineNum = kTokErr.lineNum;
        kConfigErr->colNum = kTokErr.colNum;
        kConfigErr->text = kText;
        kConfigErr->subtext = kSubtext;
    }
}

bool ConfigUtil::checkEof(const TokenIterator& kIt,
                          const Token& kTokLast,
                          const std::string kErrText,
                          ConfigErrorInfo* const kConfigErr)
{
    if (kIt.eof())
    {
        if (kConfigErr != nullptr)
        {
            kConfigErr->lineNum = kTokLast.lineNum;
            kConfigErr->colNum = (kTokLast.colNum + kTokLast.str.size());
            kConfigErr->text = kErrText;
            kConfigErr->subtext = "unexpected end of file";
        }
        return true;
    }

    return false;
}
