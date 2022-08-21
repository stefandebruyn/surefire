////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/LanguageConstants.hpp"

namespace Sf
{

const String LangConst::funcRollAvg = "roll_avg";

const String LangConst::funcRollMedian = "roll_median";

const String LangConst::funcRollMin = "roll_min";

const String LangConst::funcRollMax = "roll_max";

const String LangConst::funcRollRange = "roll_range";

const U32 LangConst::rollWindowMaxSize = 100000;

const String LangConst::elemStateTime = "T";

const String LangConst::elemGlobalTime = "G";

const String LangConst::elemState = "S";

const String LangConst::optDeltaT = "delta_t";

const String LangConst::optInitState = "init_state";

const String LangConst::labelEntry = ".entry";

const String LangConst::labelStep = ".step";

const String LangConst::labelExit = ".exit";

const String LangConst::annotationAssert = "@assert";

const String LangConst::annotationAlias = "@alias";

const String LangConst::annotationReadOnly = "@read_only";

const String LangConst::annotationStop = "@stop";

const String LangConst::sectionStateVector = "[state_vector]";

const String LangConst::sectionLocal = "[local]";

const String LangConst::sectionAllStates = "[all_states]";

const String LangConst::sectionOptions = "[options]";

const String LangConst::optLock = "lock";

const String LangConst::keywordIf = "if";

const String LangConst::keywordElse = "else";

const String LangConst::keywordTransition = "->";

const String LangConst::constantTrue = "true";

const String LangConst::constantFalse = "false";

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
};

const Map<ElementType, TypeInfo> TypeInfo::fromEnum =
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
    {OpInfo::Type::NOT, "not", "!", 11, true, false, false, true};

const OpInfo OpInfo::mult =
    {OpInfo::Type::MULT, "*", "*", 10, false, true, true, false};

const OpInfo OpInfo::divide =
    {OpInfo::Type::DIV, "/", "/", 10, false, true, true, false};

const OpInfo OpInfo::add =
    {OpInfo::Type::ADD, "+", "+", 9, false, true, true, false};

const OpInfo OpInfo::sub =
    {OpInfo::Type::SUB, "-", "-", 9, false, true, true, false};

const OpInfo OpInfo::lt =
    {OpInfo::Type::LT, "<", "<", 8, false, false, true, false};

const OpInfo OpInfo::lte =
    {OpInfo::Type::LTE, "<=", "<=", 8, false, false, true, false};

const OpInfo OpInfo::gt =
    {OpInfo::Type::GT, ">", ">", 8, false, false, true, false};

const OpInfo OpInfo::gte =
    {OpInfo::Type::GTE, ">=", ">=", 8, false, false, true, false};

const OpInfo OpInfo::eq =
    {OpInfo::Type::EQ, "==", "==", 7, false, false, true, true};

const OpInfo OpInfo::neq =
    {OpInfo::Type::NEQ, "!=", "!=", 7, false, false, true, true};

const OpInfo OpInfo::land =
    {OpInfo::Type::AND, "and", "&&", 6, false, false, false, true};

const OpInfo OpInfo::lor =
    {OpInfo::Type::OR, "or", "||", 5, false, false, false, true};

const Map<String, OpInfo> OpInfo::fromStr =
{
    {lnot.str, lnot},
    {"!", lnot}, // Alias for logical not
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

} // namespace Sf
