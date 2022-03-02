#pragma once

#include "dump_utility.hpp"

#include <map>
#include <sdbusplus/bus.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DumpType;
using ::sdbusplus::message::object_path;

/**
 * @class DumpOffloadQueue
 * @brief Queue the dump offload request and send one by one after the
 *        offload is completed. When an offload is completed the dump D-Bus
 *        object will be deleted, subscribe to that signal and then offload
 *        the next dump in the queue.
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
     * @param[in] type - type of the dump to offload
     */
    void enqueueForOffloading(const object_path& path, DumpType type);

    /**
     * @brief DeQueue the dumps from offloading
     *        Dequeue can happen after succesfull offload or when dump objects
     *        are deleted by redfish client
     * @param[in] path - D-Bus path of the dump object
     */
    void dequeueForOffloading(const object_path& path);

    /**
     * @brief Offload the next available dump from the queue
     */
    void offload();

  private:
    /** @brief D-Bus to connect to */
    sdbusplus::bus::bus& _bus;

    /** @brief map of property change request for the corresponding entry */
    std::map<std::string, DumpType> _offloadDumpList;

    /** @brief dump object currently in offload */
    std::string _offloadObjPath;
};
} // namespace openpower::dump
