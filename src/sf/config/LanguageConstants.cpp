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
