#ifndef SF_STATE_VECTOR_PARSER_HPP
#define SF_STATE_VECTOR_PARSER_HPP

#include <istream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/Tokenizer.hpp"
#include "sf/core/StateVector.hpp"

namespace StateVectorParser
{
    extern const Vec<String> ALL_REGIONS;

    struct ElementParse final
    {
        Token tokType;
        Token tokName;
    };

    struct RegionParse final
    {
        Token tokName;
        String plainName;
        Vec<StateVectorParser::ElementParse> elems;
    };

    struct Parse final
    {
        Vec<StateVectorParser::RegionParse> regions;
    };

    Result parse(const Vec<Token>& kToks,
                 StateVectorParser::Parse& kParse,
                 ErrorInfo* const kErr,
                 const Vec<String> kRgns = ALL_REGIONS);
}

#endif
