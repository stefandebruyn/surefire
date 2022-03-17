#ifndef SF_STATE_VECTOR_PARSER_HPP
#define SF_STATE_VECTOR_PARSER_HPP

#include <istream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/Tokenizer.hpp"
#include "sf/core/StateVector.hpp"

namespace StateVectorParser
{
    extern const std::vector<std::string> ALL_REGIONS;

    struct ElementParse final
    {
        Token tokType;
        Token tokName;
    };

    struct RegionParse final
    {
        Token tokName;
        std::string plainName;
        std::vector<ElementParse> elems;
    };

    struct Parse final
    {
        std::vector<RegionParse> regions;
    };

    Result parse(const std::vector<Token>& kToks,
                 Parse& kParse,
                 ErrorInfo* const kErr,
                 const std::vector<std::string> kRgns = ALL_REGIONS);
}

#endif
