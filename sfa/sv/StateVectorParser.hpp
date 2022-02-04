#ifndef SFA_STATE_VECTOR_PARSER_HPP
#define SFA_STATE_VECTOR_PARSER_HPP

#include <string>
#include <istream>
#include <vector>
#include <regex>
#include <memory>
#include <unordered_map>

#include "sfa/sv/StateVector.hpp"
#include "sfa/config/ConfigTokenizer.hpp"
#include "sfa/config/ConfigErrorInfo.hpp"

class StateVectorParser final
{
public:

    static const std::vector<std::string> ALL_REGIONS;

    struct ElementParse final
    {
        Token tokType;
        Token tokName;
        U32 sizeBytes;
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

    class Config final
    {
    public:

        Config(const StateVector::Config kSvConfig,
               const char* const kSvBacking,
               const Parse& kParse);

        ~Config();

        const StateVector::Config& get() const;

        const Parse& getParse() const;

    private:

        const StateVector::Config mSvConfig;

        const char* const mSvBacking;

        const Parse mParse;
    };

    static Result parse(const std::string kFilePath,
                        std::shared_ptr<Config>& kConfig,
                        ConfigErrorInfo* kConfigErr,
                        const std::vector<std::string> kRegions = ALL_REGIONS);

    static Result parse(std::istream& kIs,
                        std::shared_ptr<Config>& kConfig,
                        ConfigErrorInfo* kConfigErr,
                        const std::vector<std::string> kRegions = ALL_REGIONS);

    StateVectorParser() = delete;

private:

    static const std::regex mRegionSectionRegex;

    static const std::unordered_map<std::string, U32> mElemTypeSize;

    static Result parseImpl(const std::vector<Token>& kToks,
                            std::shared_ptr<Config>& kConfig,
                            ConfigErrorInfo* kConfigErr,
                            const std::vector<std::string>& kRegions);

    static Result parseRegion(const std::vector<Token>& kToks,
                              U32& kIdx,
                              const Parse& kParse,
                              RegionParse& kRegion,
                              ConfigErrorInfo* kConfigErr);

    static Result parseElement(const std::vector<Token>& kToks,
                               U32& kIdx,
                               const Parse& kParse,
                               ElementParse& kElem,
                               ConfigErrorInfo* kConfigErr);

    static Result allocateElement(const ElementParse& kElem,
                                  StateVector::ElementConfig& kElemInfo,
                                  char*& kBumpPtr);

    static Result makeConfig(const Parse& kParse,
                             ConfigErrorInfo* kConfigErr,
                             std::shared_ptr<Config>& kConfig);

};

#endif
