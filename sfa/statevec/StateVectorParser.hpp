#ifndef SFA_STATE_VECTOR_PARSER_HPP
#define SFA_STATE_VECTOR_PARSER_HPP

#include <string>
#include <istream>
#include <vector>
#include <regex>

#include "sfa/statevec/StateVector.hpp"
#include "sfa/ConfigTokenizer.hpp"

class StateVectorParser final
{
public:

    StateVectorParser() = delete;

    static Result parse(const std::string kFilePath,
                        StateVector::Config& kConfig,
                        ConfigErrorInfo* kErrInfo);

    static Result parse(std::istream& kIs,
                        StateVector::Config& kConfig,
                        ConfigErrorInfo* kErrInfo);

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
                            ConfigErrorInfo* kErrInfo);

    static Result parseRegion(const std::vector<Token>& kToks,
                              U32& kIdx,
                              RegionParse& kRegion,
                              ConfigErrorInfo* kErrInfo);

    static Result parseElement(const std::vector<Token>& kToks,
                               U32& kIdx,
                               ElementParse& kElem,
                               ConfigErrorInfo* kErrInfo);
};

#endif
