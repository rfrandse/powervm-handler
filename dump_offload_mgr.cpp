#include "config.h"

#include "dump_offload_mgr.hpp"

#include "dump_dbus_util.hpp"
namespace openpower::dump
{
DumpOffloadManager::DumpOffloadManager(sdbusplus::bus::bus& bus,
                                       sdeventplus::Event& event) :
    _bus(bus),
    _dumpQueue(bus, event), _hostStateWatch(bus, _dumpQueue),
    _hmcStateWatch(bus, _dumpQueue)
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
}

void DumpOffloadManager::offload()
{
    for (auto& dump : _dumpOffloadHandlerList)
    {
        dump->offload();
    }
}
} // namespace openpower::dump
