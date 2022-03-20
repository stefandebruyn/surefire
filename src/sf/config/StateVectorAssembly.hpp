#ifndef SF_STATE_VECTOR_ASSEMBLY_HPP
#define SF_STATE_VECTOR_ASSEMBLY_HPP

#include <istream>

#include "sf/config/StateVectorParse.hpp"

class StateVectorAssembly final
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

    ~StateVectorAssembly();

    Ref<StateVector> get() const;

    StateVector::Config config() const;

    Ref<const StateVectorParse> parse() const;

private:

    const Ref<StateVector> mObj;

    const StateVector::Config mConfig;

    const Ref<const StateVectorParse> mParse;

    const char* const mBacking;

    static void allocateElement(const StateVectorParse::ElementParse& kElem,
                                StateVector::ElementConfig& kElemInfo,
                                char*& kBumpPtr);

    StateVectorAssembly(const Ref<StateVector> kObj,
                        const StateVector::Config kConfig,
                        const Ref<const StateVectorParse> kParse,
                        const char* const kBacking);
};

#endif
