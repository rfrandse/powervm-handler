#pragma once

#include "dump_watch.hpp"
#include "host_offloader_queue.hpp"
#include "utility.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace openpower::dump
{

/**
 * @class OffloadHandler
 * @brief Class to offload dumps to PHYP service patition using PLDM
 * @details Fetch all the completed dumps present and initiate PLDM command
 *      to send dump offload requests to server. Add watch to watch on
 *      new dumps created to cater for offload.
 */

class OffloadHandler
{
  public:
    OffloadHandler() = delete;
    virtual ~OffloadHandler() = default;
    OffloadHandler(const OffloadHandler&) = delete;
    OffloadHandler& operator=(const OffloadHandler&) = delete;
    OffloadHandler(OffloadHandler&&) = delete;
    OffloadHandler& operator=(OffloadHandler&&) = delete;

    /**
     * @brief constructor
     * @param[in] bus - D-Bus handle
     * @param[in] offloader - To queue and offload dump
     * @param[in] entryIntf - entry interface to watch
     * @param[in] entryObjPath - entry object path to watch
     * @param[in] dumpType - type of the dump to watch
     */
    OffloadHandler(sdbusplus::bus::bus& bus, HostOffloaderQueue& offloader,
                   const std::string& entryIntf,
                   const std::string& entryObjPath, DumpType dumpType);

    /**
     * @brief Offload dump by sending request to PLDM
     * @param[in] existing dump objects
     */
    void offload();

  protected:
    /* @brief sdbusplus DBus bus connection. */
    sdbusplus::bus::bus& _bus;

    /** @brief Queue to offload dump requests */
    HostOffloaderQueue& _dumpOffloader;

    /* @brief entry interface this object supports */
    const std::string _entryIntf;

    /* @brief dump type this object supports */
    const DumpType _dumpType;

    /* @brief watch on interfaces added/removed and property */
    openpower::dump::DumpWatch _dumpWatch;
};
} // namespace openpower::dump
