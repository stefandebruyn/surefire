#ifndef SF_NI_FPGA_SESSION_HPP
#define SF_NI_FPGA_SESSION_HPP

#include "nifpga/NiFpga.h"
#include "nifpga/NiFpga_IO.h"
#include "sf/core/Result.hpp"

Result niFpgaSessionOpen(NiFpga_Session& kSession);

Result niFpgaSessionClose(const NiFpga_Session kSession);

#endif
