#pragma once
#include "dump_offload_queue.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace openpower::dump
{

/**
 * @class DumpHostStateWatch
 * @brief Add watch on host state change to offload dumps
 */
class DumpHostStateWatch
{
  public:
    DumpHostStateWatch() = delete;
    DumpHostStateWatch(const DumpHostStateWatch&) = delete;
    DumpHostStateWatch& operator=(const DumpHostStateWatch&) = delete;
    DumpHostStateWatch(DumpHostStateWatch&&) = delete;
    DumpHostStateWatch& operator=(DumpHostStateWatch&&) = delete;
    virtual ~DumpHostStateWatch() = default;

    /**
     * @brief Watch on new host state change
     * @param[in] bus - Bus to attach to
     * @param[in] dumpQueue - dump queue
     */
    DumpHostStateWatch(sdbusplus::bus::bus& bus, DumpOffloadQueue& dumpQueue);

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
    DumpOffloadQueue& _dumpQueue;

    /*@brief watch for host state change */
    std::unique_ptr<sdbusplus::bus::match_t> _hostStatePropWatch;
};
} // namespace openpower::dump
