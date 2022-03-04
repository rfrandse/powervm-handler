#pragma once
#include "host_offloader_queue.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace openpower::dump
{

/**
 * @class HMCStateWatch
 * @brief Add watch on HMC state change to offload dumps, DUMPS are offloaded
 * only for non HMC systems
 */
class HMCStateWatch
{
  public:
    HMCStateWatch() = delete;
    HMCStateWatch(const HMCStateWatch&) = delete;
    HMCStateWatch& operator=(const HMCStateWatch&) = delete;
    HMCStateWatch(HMCStateWatch&&) = delete;
    HMCStateWatch& operator=(HMCStateWatch&&) = delete;
    virtual ~HMCStateWatch() = default;

    /**
     * @brief Watch on new HMC state change
     * @param[in] bus - Bus to attach to
     * @param[in] dumpQueue - dump queue
     */
    HMCStateWatch(sdbusplus::bus::bus& bus, HostOffloaderQueue& dumpQueue);

  private:
    /**
     * @brief Callback method for property change on the hmc state object
     * @param[in] msg response msg from D-Bus request
     * @return void
     */
    void propertyChanged(sdbusplus::message::message& msg);

    /** @brief D-Bus to connect to */
    sdbusplus::bus::bus& _bus;

    /** @brief Queue to offload dump requests */
    HostOffloaderQueue& _dumpQueue;

    /*@brief watch for hmc state change */
    std::unique_ptr<sdbusplus::bus::match_t> _hmcStatePropWatch;
};
} // namespace openpower::dump
