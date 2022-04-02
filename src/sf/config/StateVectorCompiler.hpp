#ifndef SF_STATE_VECTOR_COMPILER_HPP
#define SF_STATE_VECTOR_COMPILER_HPP

#include <istream>

#include "sf/config/StateVectorParser.hpp"

class StateVectorAssembly final
{
public:

    StateVector& get() const;

    StateVector::Config config() const;

    Ref<const StateVectorParse> parse() const;

private:

    friend class StateVectorCompiler;

    struct Workspace final
    {
        Ref<Vec<StateVector::ElementConfig>> elemConfigs;
        Ref<Vec<StateVector::RegionConfig>> regionConfigs;
        Ref<Vec<U8>> svBacking;
        Vec<Ref<String>> configStrings;
        Vec<Ref<IElement>> elems;
        Vec<Ref<Region>> regions;

        Ref<StateVector> sv;
        StateVector::Config svConfig;
        Ref<const StateVectorParse> svParse;
    };

    const StateVectorAssembly::Workspace mWs;

    StateVectorAssembly(const StateVectorAssembly::Workspace& kWs);
};

class StateVectorCompiler final
{
public:

    static Result compile(const String kFilePath,
                          Ref<const StateVectorAssembly>& kAsm,
                          ErrorInfo* const kErr);

    static Result compile(std::istream& kIs,
                          Ref<const StateVectorAssembly>& kAsm,
                          ErrorInfo* const kErr);

    static Result compile(const Ref<const StateVectorParse> kParse,
                          Ref<const StateVectorAssembly>& kAsm,
                          ErrorInfo* const kErr);

    StateVectorCompiler() = delete;

private:

    static Result allocateElement(const StateVectorParse::ElementParse& kElem,
                                  StateVectorAssembly::Workspace& kWs,
                                  StateVector::ElementConfig& kElemConfig,
                                  U8*& kBumpPtr);
};

#endif