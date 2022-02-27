#include <algorithm>

#include "sfa/sup/ConfigUtil.hpp"

namespace ConfigUtil
{
    const ElementTypeInfo i8Info =
        {ElementType::INT8, "I8", {}, 1, true, false, true};

    const ElementTypeInfo i16Info =
        {ElementType::INT16, "I16", {}, 2, true, false, true};

    const ElementTypeInfo i32Info =
        {ElementType::INT32, "I32", {}, 4, true, false, true};

    const ElementTypeInfo i64Info =
        {ElementType::INT64, "I64", {}, 8, true, false, true};

    const ElementTypeInfo u8Info =
        {ElementType::UINT8, "U8", {}, 1, true, false, false};

    const ElementTypeInfo u16Info =
        {ElementType::UINT16, "U16", {}, 2, true, false, false};

    const ElementTypeInfo u32Info =
        {ElementType::UINT32, "U32", {}, 4, true, false, false};

    const ElementTypeInfo u64Info =
        {ElementType::UINT64, "U64", {}, 8, true, false, false};

    const ElementTypeInfo f32Info =
        {ElementType::FLOAT32, "F32", {}, 4, true, true, true};

    const ElementTypeInfo f64Info =
        {ElementType::FLOAT64, "F64", {}, 8, true, true, true};

    const ElementTypeInfo boolInfo =
        {ElementType::BOOL, "bool", {"BOOL"}, 1, false, false, false};
}

const std::unordered_map<std::string, ConfigUtil::ElementTypeInfo>
    ConfigUtil::typeInfoFromName =
{
    {i8Info.name, i8Info},
    {i16Info.name, i16Info},
    {i32Info.name, i32Info},
    {i64Info.name, i64Info},
    {u8Info.name, u8Info},
    {u16Info.name, u16Info},
    {u32Info.name, u32Info},
    {u64Info.name, u64Info},
    {f32Info.name, f32Info},
    {f64Info.name, f64Info},
    {boolInfo.name, boolInfo},
    {"BOOL", boolInfo}
};

const std::unordered_map<ElementType, ConfigUtil::ElementTypeInfo, EnumHash>
    ConfigUtil::typeInfoFromEnum =
{
    {i8Info.enumVal, i8Info},
    {i16Info.enumVal, i16Info},
    {i32Info.enumVal, i32Info},
    {i64Info.enumVal, i64Info},
    {u8Info.enumVal, u8Info},
    {u16Info.enumVal, u16Info},
    {u32Info.enumVal, u32Info},
    {u64Info.enumVal, u64Info},
    {f32Info.enumVal, f32Info},
    {f64Info.enumVal, f64Info},
    {boolInfo.enumVal, boolInfo}
};

const std::unordered_set<std::string> ConfigUtil::reserved =
{
    "STATE_VECTOR",
    "LOCAL",
    "IF",
    "ELSE",
    "NOT",
    "ENTRY",
    "STEP",
    "EXIT",
    "T",
    "G",
    "S"
};
