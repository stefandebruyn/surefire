#ifndef SFA_STATE_VECTOR_PARSER_HPP
#define SFA_STATE_VECTOR_PARSER_HPP

#include <string>
#include <istream>
#include <vector>
#include <regex>

#include "sfa/statevec/StateVector.hpp"
#include "sfa/ConfigTokenizer.hpp"
#include "sfa/ConfigInfo.hpp"

class StateVectorParser final
{
public:

    StateVectorParser() = delete;

    static Result parse(const std::string kFilePath,
                        StateVector::Config& kConfig,
                        ConfigInfo* kConfigInfo);

    static Result parse(std::istream& kIs,
                        StateVector::Config& kConfig,
                        ConfigInfo* kConfigInfo);

private:

    static const std::regex mRegionSectionRegex;

    struct ElementParse
    {
        std::string type;
        std::string name;
    };

    struct RegionParse
    {
        std::string name;
        std::vector<ElementParse> elems;
    };

    struct StateVectorParse
    {
        std::vector<RegionParse> regions;
        U32 svSizeBytes;
    };

    static Result parseImpl(const std::vector<Token>& kToks,
                            StateVector::Config& kConfig,
                            ConfigInfo* kConfigInfo);

    static Result parseRegion(const std::vector<Token>& kToks,
                              U32& kIdx,
                              RegionParse& kRegion,
                              ConfigInfo* kConfigInfo);

    static Result parseElement(const std::vector<Token>& kToks,
                               U32& kIdx,
                               ElementParse& kElem,
                               ConfigInfo* kConfigInfo);
};

#endif
