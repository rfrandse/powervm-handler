#include "config.h"

#include "offload_manager.hpp"

#include "dbus_util.hpp"
namespace openpower::dump
{
OffloadManager::OffloadManager(sdbusplus::bus::bus& bus,
                               sdeventplus::Event& event) :
    _bus(bus),
    _dumpQueue(bus, event), _hostStateWatch(bus, _dumpQueue),
    _hmcStateWatch(bus, _dumpQueue)
{

    // add bmc dump offload handler to the list of dump types to offload
    std::unique_ptr<OffloadHandler> bmcDump = std::make_unique<OffloadHandler>(
        _bus, _dumpQueue, bmcEntryIntf, bmcEntryObjPath, DumpType::bmc);
    _offloadHandlerList.push_back(std::move(bmcDump));

    // add host dump offload handler to the list of dump types to offload
    std::unique_ptr<OffloadHandler> hostbootDump =
        std::make_unique<OffloadHandler>(_bus, _dumpQueue, hostbootEntryIntf,
                                         hostbootEntryObjPath,
                                         DumpType::hostboot);
    _offloadHandlerList.push_back(std::move(hostbootDump));

    // add sbe dump offload handler to the list of dump types to offload
    std::unique_ptr<OffloadHandler> sbeDump = std::make_unique<OffloadHandler>(
        _bus, _dumpQueue, sbeEntryIntf, sbeEntryObjPath, DumpType::sbe);
    _offloadHandlerList.push_back(std::move(sbeDump));

    // add hardware dump offload handler to the list of dump types to
    // offload
    std::unique_ptr<OffloadHandler> hardwareDump =
        std::make_unique<OffloadHandler>(_bus, _dumpQueue, hardwareEntryIntf,
                                         hardwareEntryObjPath,
                                         DumpType::hardware);
    _offloadHandlerList.push_back(std::move(hardwareDump));
}

void OffloadManager::offload()
{
    for (auto& dump : _offloadHandlerList)
    {
        dump->offload();
    }
}
} // namespace openpower::dump
