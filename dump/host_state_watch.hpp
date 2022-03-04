#pragma once
#include "host_offloader_queue.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace openpower::dump
{

/**
 * @class HostStateWatch
 * @brief Add watch on host state change to offload dumps
 */
class HostStateWatch
{
  public:
    HostStateWatch() = delete;
    HostStateWatch(const HostStateWatch&) = delete;
    HostStateWatch& operator=(const HostStateWatch&) = delete;
    HostStateWatch(HostStateWatch&&) = delete;
    HostStateWatch& operator=(HostStateWatch&&) = delete;
    virtual ~HostStateWatch() = default;

    /**
     * @brief Watch on new host state change
     * @param[in] bus - Bus to attach to
     * @param[in] dumpQueue - dump queue
     */
    HostStateWatch(sdbusplus::bus::bus& bus, HostOffloaderQueue& dumpQueue);

  private:
    /**
     * @brief Callback method for property change on the host state object
     * @param[in] msg response msg from D-Bus request
     * @return void
     */
    void propertyChanged(sdbusplus::message::message& msg);

    /** @brief D-Bus to connect to */
    sdbusplus::bus::bus& _bus;

    /** @brief Queue to offload dump requests */
    HostOffloaderQueue& _dumpQueue;

    /*@brief watch for host state change */
    std::unique_ptr<sdbusplus::bus::match_t> _hostStatePropWatch;
};
} // namespace openpower::dump
