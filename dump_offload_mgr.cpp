#include "config.h"

#include "dump_offload_mgr.hpp"

#include "dump_dbus_util.hpp"
namespace openpower::dump
{
DumpOffloadManager::DumpOffloadManager(sdbusplus::bus::bus& bus) :
    _bus(bus), _dumpQueue(bus)
{
    // add bmc dump offload handler to the list of dump types to offload
    std::unique_ptr<DumpOffloadHandler> bmcDump =
        std::make_unique<DumpOffloadHandler>(_bus, _dumpQueue, bmcEntryIntf,
                                             bmcEntryObjPath, DumpType::bmc);
    _dumpOffloadHandlerList.push_back(std::move(bmcDump));

    // add host dump offload handler to the list of dump types to offload
    std::unique_ptr<DumpOffloadHandler> hostbootDump =
        std::make_unique<DumpOffloadHandler>(
            _bus, _dumpQueue, hostbootEntryIntf, hostbootEntryObjPath,
            DumpType::hostboot);
    _dumpOffloadHandlerList.push_back(std::move(hostbootDump));

    // add sbe dump offload handler to the list of dump types to offload
    std::unique_ptr<DumpOffloadHandler> sbeDump =
        std::make_unique<DumpOffloadHandler>(_bus, _dumpQueue, sbeEntryIntf,
                                             sbeEntryObjPath, DumpType::sbe);
    _dumpOffloadHandlerList.push_back(std::move(sbeDump));

    // add hardware dump offload handler to the list of dump types to
    // offload
    std::unique_ptr<DumpOffloadHandler> hardwareDump =
        std::make_unique<DumpOffloadHandler>(
            _bus, _dumpQueue, hardwareEntryIntf, hardwareEntryObjPath,
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
        [this](auto& msg) { this->hostStatePropChanged(msg); });
}

void DumpOffloadManager::hostStatePropChanged(sdbusplus::message::message& msg)
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
                    // offload the already queued dumps now
                    _dumpQueue.offload();
                }
            }
        }
    }
}

void DumpOffloadManager::offload()
{
    for (auto& dump : _dumpOffloadHandlerList)
    {
        dump->offload();
    }
}
} // namespace openpower::dump
