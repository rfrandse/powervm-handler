#pragma once

#include "dump_host_state_watch.hpp"
#include "dump_offload_handler.hpp"
#include "dump_offload_queue.hpp"
#include "hmc_state_watch.hpp"

#include <memory>
#include <sdbusplus/bus.hpp>
#include <sdeventplus/source/event.hpp>

namespace openpower::dump
{
/**
 * @class DumpOffloadManager
 * @brief To offload dumps to PHYP service parition by using PLDM commands
 * @details Retrieves all the suported dumps entries and initiates PLDM
 *         request to offload the dumps to PHYP service partition
 */
class DumpOffloadManager
{
  public:
    DumpOffloadManager() = delete;
    DumpOffloadManager(const DumpOffloadManager&) = delete;
    DumpOffloadManager& operator=(const DumpOffloadManager&) = delete;
    DumpOffloadManager(DumpOffloadManager&&) = delete;
    DumpOffloadManager& operator=(DumpOffloadManager&&) = delete;
    virtual ~DumpOffloadManager() = default;

    /**
     * @brief Constructor
     * @param[in] bus - D-Bus to attach to.
     * @param[in] event - event handler
     */
    DumpOffloadManager(sdbusplus::bus::bus& bus, sdeventplus::Event& event);

    /**
     * @brief Offload dumps existing on the system by sending PLDM request
     */
    void offload();

  private:
    /** @brief D-Bus to connect to */
    sdbusplus::bus::bus& _bus;

    /** @brief Queue to offload dump requests */
    DumpOffloadQueue _dumpQueue;

    /*@brief list of dump offload objects */
    std::vector<std::unique_ptr<DumpOffloadHandler>> _dumpOffloadHandlerList;

    /*@brief watch for host state change */
    DumpHostStateWatch _hostStateWatch;

    /*@brief watch for HMC state change */
    HMCStateWatch _hmcStateWatch;
};
} // namespace openpower::dump
