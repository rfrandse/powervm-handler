#include "config.h"

#include "dump_offload_mgr.hpp"

#include "dump_dbus_util.hpp"

namespace openpower::dump
{
DumpOffloadManager::DumpOffloadManager(sdbusplus::bus::bus& bus) : _bus(bus)
{
    // add bmc dump offload handler to the list of dump types to offload
    std::unique_ptr<DumpOffloadHandler> bmcDump =
        std::make_unique<DumpOffloadHandler>(_bus, bmcEntryIntf, DumpType::bmc);
    _dumpOffloadHandlerList.push_back(std::move(bmcDump));

    // add host dump offload handler to the list of dump types to offload
    std::unique_ptr<DumpOffloadHandler> hostbootDump =
        std::make_unique<DumpOffloadHandler>(_bus, hostbootEntryIntf,
                                             DumpType::hostboot);
    _dumpOffloadHandlerList.push_back(std::move(hostbootDump));

    // add sbe dump offload handler to the list of dump types to offload
    std::unique_ptr<DumpOffloadHandler> sbeDump =
        std::make_unique<DumpOffloadHandler>(_bus, sbeEntryIntf, DumpType::sbe);
    _dumpOffloadHandlerList.push_back(std::move(sbeDump));

    // add hardware dump offload handler to the list of dump types to
    // offload
    std::unique_ptr<DumpOffloadHandler> hardwareDump =
        std::make_unique<DumpOffloadHandler>(_bus, hardwareEntryIntf,
                                             DumpType::hardware);
    _dumpOffloadHandlerList.push_back(std::move(hardwareDump));

    // Do not offload when host is not in running state so adding watch on
    // Boot progress property.
    //
    // Host might go from running state to unspecified state and come back
    // and is independent of this service, so when host comes backs to runtime
    // any dumps that are ignored when host is offline should be offloaded.
    //
    _hostStatePropWatch = std::make_unique<sdbusplus::bus::match_t>(
        _bus,
        sdbusplus::bus::match::rules::propertiesChanged(
            "/xyz/openbmc_project/state/host0",
            "xyz.openbmc_project.State.Boot.Progress"),
        [this](auto& msg) { this->propertiesChanged(msg); });
}

void DumpOffloadManager::propertiesChanged(sdbusplus::message::message& msg)
{
    std::string intf;
    DBusPropertiesMap propMap;
    msg.read(intf, propMap);
    log<level::INFO>(
        fmt::format("Host state propertiesChanged interface ({}) ", intf)
            .c_str());
    for (auto prop : propMap)
    {
        if (prop.first == "BootProgress")
        {
            auto progress = std::get_if<std::string>(&prop.second);
            if (progress != nullptr)
            {
                ProgressStages bootProgress =
                    sdbusplus::xyz::openbmc_project::State::Boot::server::
                        Progress::convertProgressStagesFromString(*progress);
                if ((bootProgress == ProgressStages::SystemInitComplete) ||
                    (bootProgress == ProgressStages::OSStart) ||
                    (bootProgress == ProgressStages::OSRunning))
                {
                    log<level::INFO>(
                        fmt::format("Offloading dumps host is now in  ({})"
                                    " state ",
                                    *progress)
                            .c_str());
                    offloadHelper();
                }
            }
        }
    }
}

void DumpOffloadManager::offloadHelper()
{
    // HMC might go from Non-hmc to HMC managed system so off load only
    // if it is non HMC
    if (!isSystemHMCManaged(_bus))
    {
        // we can query only on the dump service not on individual entry types,
        // so we get dumps of all types
        ManagedObjectType objects = openpower::dump::getDumpEntries(_bus);

        // offload only if there are any non offloaded dumps
        if (objects.size() > 0)
        {
            for (auto& dump : _dumpOffloadHandlerList)
            {
                dump->offload(objects);
            }
        }
    }
}

void DumpOffloadManager::offload()
{
    if (isHostRunning(_bus))
    {
        offloadHelper();
    }
}
} // namespace openpower::dump
