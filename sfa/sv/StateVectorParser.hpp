#ifndef SFA_STATE_VECTOR_PARSER_HPP
#define SFA_STATE_VECTOR_PARSER_HPP

#include <string>
#include <istream>
#include <vector>
#include <regex>
#include <memory>
#include <unordered_map>

#include "sfa/sv/StateVector.hpp"
#include "sfa/ConfigTokenizer.hpp"
#include "sfa/ConfigErrorInfo.hpp"

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
                        ConfigErrorInfo* kConfigErr);

    static Result parse(std::istream& kIs,
                        std::shared_ptr<Config>& kConfig,
                        ConfigErrorInfo* kConfigErr);

private:

    static const std::regex mRegionSectionRegex;

    static const std::unordered_map<std::string, U32> mElemTypeSize;

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

    static Result parseImpl(const std::vector<Token>& kToks,
                            std::shared_ptr<Config>& kConfig,
                            ConfigErrorInfo* kConfigErr);

    static Result parseRegion(const std::vector<Token>& kToks,
                              U32& kIdx,
                              Parse& kParse,
                              RegionParse& kRegion,
                              ConfigErrorInfo* kConfigErr);

    static Result parseElement(const std::vector<Token>& kToks,
                               U32& kIdx,
                               Parse& kParse,
                               ElementParse& kElem,
                               ConfigErrorInfo* kConfigErr);

    static Result allocateElement(const ElementParse& kElem,
                                  StateVector::ElementConfig& kElemInfo,
                                  char*& kBumpPtr);
};

#endif
