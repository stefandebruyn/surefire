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

    Result parse(const std::string kFilePath,
                 std::shared_ptr<Config>& kConfig,
                 ConfigErrorInfo* kConfigErr,
                 const std::vector<std::string> kRegions = ALL_REGIONS);

    Result parse(std::istream& kIs,
                 std::shared_ptr<Config>& kConfig,
                 ConfigErrorInfo* kConfigErr,
                 const std::vector<std::string> kRegions = ALL_REGIONS);
}

#endif
