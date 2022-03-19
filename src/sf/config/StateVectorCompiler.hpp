#ifndef SF_STATE_VECTOR_COMPILER_HPP
#define SF_STATE_VECTOR_COMPILER_HPP

#include <istream>

#include "sf/config/StateVectorParser.hpp"

namespace StateVectorCompiler
{
    class Assembly final
    {
    public:

        Assembly(const StateVector::Config kSvConfig,
                 const char* const kSvBacking,
                 const StateVectorParser::Parse& kParse);

        ~Assembly();

        const StateVector::Config& getConfig() const;

        const StateVectorParser::Parse& getParse() const;

    private:

        const StateVector::Config mSvConfig;

        const char* const mSvBacking;

        const StateVectorParser::Parse mParse;
    };

    Result compile(const std::string kFilePath,
                   std::shared_ptr<Assembly>& kAsm,
                   ErrorInfo* const kErr);

    Result compile(std::istream& kIs,
                   std::shared_ptr<Assembly>& kAsm,
                   ErrorInfo* const kErr);

    Result compile(const StateVectorParser::Parse& kParse,
                   std::shared_ptr<Assembly>& kAsm,
                   ErrorInfo* const kErr);
}

#endif