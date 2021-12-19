#ifndef SFA_STATE_VECTOR_PARSER_HPP
#define SFA_STATE_VECTOR_PARSER_HPP

#include <string>
#include <istream>
#include <vector>
#include <regex>
#include <memory>
#include <unordered_map>

#include "sfa/statevec/StateVector.hpp"
#include "sfa/ConfigTokenizer.hpp"
#include "sfa/ConfigInfo.hpp"

class StateVectorParser final
{
public:

    class Config final
    {
    public:

        Config(const StateVector::Config kSvConfig,
               const char* const kSvBacking);

        ~Config();

        const StateVector::Config& get() const;

    private:

        const StateVector::Config mSvConfig;

        const char* const mSvBacking;
    };

    StateVectorParser() = delete;

    static Result parse(const std::string kFilePath,
                        std::shared_ptr<Config>& kConfig,
                        ConfigInfo* kConfigInfo);

    static Result parse(std::istream& kIs,
                        std::shared_ptr<Config>& kConfig,
                        ConfigInfo* kConfigInfo);

private:

    static const std::regex mRegionSectionRegex;

    static const std::unordered_map<std::string, U32> mElemTypeSize;

    struct ElementParse
    {
        Token tokType;
        Token tokName;
    };

    struct RegionParse
    {
        Token tokName;
        std::string plainName;
        std::vector<ElementParse> elems;
    };

    struct StateVectorParse
    {
        std::vector<RegionParse> regions;
    };

    static Result parseImpl(const std::vector<Token>& kToks,
                            std::shared_ptr<Config>& kConfig,
                            ConfigInfo* kConfigInfo);

    static Result parseRegion(const std::vector<Token>& kToks,
                              U32& kIdx,
                              RegionParse& kRegion,
                              ConfigInfo* kConfigInfo);

    static Result parseElement(const std::vector<Token>& kToks,
                               U32& kIdx,
                               ElementParse& kElem,
                               ConfigInfo* kConfigInfo);

    static Result allocateElement(const ElementParse& kElem,
                                  StateVector::ElementInfo& kElemInfo,
                                  char*& kBumpPtr);
};

#endif
