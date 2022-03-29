#ifndef SF_STATE_MACHINE_AUTOCODER_HPP
#define SF_STATE_MACHINE_AUTOCODER_HPP

#include "sf/config/StateMachineAssembly.hpp"

namespace StateMachineAutocoder
{
    Result code(std::ostream& kOs,
                const String kName,
                const Ref<const StateMachineAssembly> kAsm);
}

#endif
