#ifndef SFA_STATE_VECTOR_PARSER_HPP
#define SFA_STATE_VECTOR_PARSER_HPP

#include <string>
#include <istream>
#include <vector>
#include <memory>
#include <unordered_map>

#include "sfa/core/StateVector.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"
#include "sfa/sup/ConfigErrorInfo.hpp"

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
                 ConfigErrorInfo* kConfigErr,
                 const std::vector<std::string> kRgns = ALL_REGIONS);
}

#endif
