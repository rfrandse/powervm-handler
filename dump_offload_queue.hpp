#pragma once

#include "dump_utility.hpp"

#include <map>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DumpType;
using ::sdbusplus::message::object_path;

/**
 * @class DumpOffloadQueue
 * @brief Queue to send dump offload requests when one offload is completed
 * @details PHYP could not handle multiple dump offload requests at the same
 *          time, queueing the requests and sending when one offload is done
 */
class DumpOffloadQueue
{
  public:
    DumpOffloadQueue() = delete;
    DumpOffloadQueue(const DumpOffloadQueue&) = delete;
    DumpOffloadQueue& operator=(const DumpOffloadQueue&) = delete;
    DumpOffloadQueue(DumpOffloadQueue&&) = delete;
    DumpOffloadQueue& operator=(DumpOffloadQueue&&) = delete;
    virtual ~DumpOffloadQueue() = default;

    /**
     * @brief Constructor
     * @param[in] bus - D-Bus to attach to
     */
    DumpOffloadQueue(sdbusplus::bus::bus& bus);

    /**
     * @brief Queue the dumps for offloading
     * @param[in] path - D-Bus path of the dump object
     * @param[in] path - type of the dump to offload
     */
    void enqueueForOffloading(const object_path& path, DumpType type);

  private:
    /**
     * @brief Offload the next available dump from the queue
     */
    void offload();

    /**
     * @brief Callback method for deletion of dump entry object
     * @param[in] msg response msg from D-Bus request
     * @return void
     */
    void interfaceRemoved(sdbusplus::message::message& msg);

    /** @brief D-Bus to connect to */
    sdbusplus::bus::bus& _bus;

    /** @brief map of property change request for the corresponding entry */
    std::map<std::string, DumpType> _offloadDumpList;

    /** @brief dump object currently in offload */
    std::string _offloadObjPath;

    /** @brief watch pointer for interfaces removed */
    std::unique_ptr<sdbusplus::bus::match_t> _intfRemWatch;
};
} // namespace openpower::dump
