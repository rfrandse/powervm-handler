#pragma once

#include "dump_utility.hpp"

namespace openpower::dump::pldm
{
using ::openpower::dump::utility::DumpType;

/**
 * @brief Send new dump offload command to PLDM
 * @param[in] dumpId ID of the dump to offload
 * @param[in] dumpType type of the dump
 * @param[in] dumpSize size of the dump to offload
 * @return
 */
void sendNewDumpCmd(uint32_t dumpId, DumpType dumpType, uint64_t dumpSize);
} // namespace openpower::dump::pldm
