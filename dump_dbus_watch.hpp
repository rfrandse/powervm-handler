#pragma once

#include "dump_offload_queue.hpp"
#include "dump_utility.hpp"

#include <map>
#include <memory>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DumpType;
using ::openpower::dump::utility::ManagedObjectType;
using ::sdbusplus::message::object_path;

/**
 * @class DumpDBusWatch
 * @brief Add watch on new dump entries created so as to offload
 * @details Adds watch on the dump progress property for the newly created
 *  dumps. Initiates offload when dump progress property is changed to complete
 */
class DumpDBusWatch
{
  public:
    DumpDBusWatch() = delete;
    DumpDBusWatch(const DumpDBusWatch&) = delete;
    DumpDBusWatch& operator=(const DumpDBusWatch&) = delete;
    DumpDBusWatch(DumpDBusWatch&&) = delete;
    DumpDBusWatch& operator=(DumpDBusWatch&&) = delete;
    virtual ~DumpDBusWatch() = default;

    /**
     * @brief Watch on new dump objects created and property change
     * @param[in] bus - Bus to attach to
     * @param[in] offloader - To queue and offload dump
     * @param[in] entryIntf - dump entry interface (BMC/Host/SBE/Hardware)
     * @param[in] dumpType - dump type to watch
     */
    DumpDBusWatch(sdbusplus::bus::bus& bus, DumpOffloadQueue& offloader,
                  const std::string& entryIntf, DumpType dumpType);

    /**
     * @brief Add all in progress dumps to property watch
     * @param[in] objects dump objects whose progress is not yet completed
     * @return void
     */
    void addInProgressDumpsToWatch(const ManagedObjectType& objects);

  private:
    /**
     * @brief Callback method for creation of dump entry object
     * @param[in] msg response msg from D-Bus request
     * @return void
     */
    void interfaceAdded(sdbusplus::message::message& msg);

    /**
     * @brief Callback method for deletion of dump entry object
     * @param[in] msg response msg from D-Bus request
     * @return void
     */
    void interfaceRemoved(sdbusplus::message::message& msg);

    /**
     * @brief Callback method for property change on the entry object
     * @param[in] objPath Object path of the dump entry
     * @param[in] id ID of the dump entry
     * @param[in] msg response msg from D-Bus request
     * @return void
     */
    void propertiesChanged(const object_path& objPath, uint32_t id,
                           sdbusplus::message::message& msg);

    /** @brief D-Bus to connect to */
    sdbusplus::bus::bus& _bus;

    /** @brief Queue to offload dump requests */
    DumpOffloadQueue& _dumpOffloader;

    /** @brief entry interface to watch for */
    std::string _entryIntf;

    /** @brief type of the dump to watch for */
    DumpType _dumpType;

    /** @brief watch pointer for interfaces added */
    std::unique_ptr<sdbusplus::bus::match_t> _intfAddWatch;

    /** @brief watch pointer for interfaces removed */
    std::unique_ptr<sdbusplus::bus::match_t> _intfRemWatch;

    /** @brief map of property change request for the corresponding entry */
    std::map<object_path, std::unique_ptr<sdbusplus::bus::match_t>>
        _entryPropWatchList;
};
} // namespace openpower::dump
