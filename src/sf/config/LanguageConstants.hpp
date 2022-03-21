#ifndef SF_LANGUAGE_CONSTANTS_HPP
#define SF_LANGUAGE_CONSTANTS_HPP

#include <regex>

#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"

namespace LangConst
{
    extern const String funcNameRollAvg;

    extern const String funcNameRollMedian;

    extern const String funcNameRollMin;

    extern const String funcNameRollMax;

    extern const String funcNameRollRange;

    extern const U32 rollWindowMaxSize;

    extern const String elemNameStateTime;

    extern const String elemNameGlobalTime;

    extern const String elemNameState;

    extern const String labelEntry;

    extern const String labelStep;

    extern const String labelExit;

    extern const String annotationAssert;

    extern const std::regex annotationAliasRegex;

    extern const String annotationReadOnly;

    extern const String annotationStop;

    extern const String sectionStateVector;

    extern const String sectionLocal;

    extern const String keywordIf;

    extern const String keywordElse;

    extern const String keywordTransition;

    extern const String constantTrue;

    extern const String constantFalse;
}

#endif
