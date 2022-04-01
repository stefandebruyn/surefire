#ifndef SF_STATE_VECTOR_AUTOCODER_HPP
#define SF_STATE_VECTOR_AUTOCODER_HPP

#include "sf/config/StateVectorCompiler.hpp"

namespace StateVectorAutocoder
{
    Result code(std::ostream& kOs,
                const String kName,
                const Ref<const StateVectorAssembly> kSvAsm);
}

#endif
