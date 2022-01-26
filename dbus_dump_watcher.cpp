#include "config.h"

#include "dbus_dump_watcher.hpp"

#include "dump_dbus_util.hpp"
#include "dump_send_pldm_cmd.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DBusInteracesMap;
using ::openpower::dump::utility::DBusPropertiesMap;
using ::openpower::dump::utility::ManagedObjectType;
using ::phosphor::logging::level;
using ::phosphor::logging::log;
using ::sdbusplus::bus::match::rules::sender;

DBusDumpWatcher::DBusDumpWatcher(sdbusplus::bus::bus& bus,
                                 const std::string& entryIntf,
                                 DumpType dumpType) :
    _bus(bus),
    _entryIntf(entryIntf), _dumpType(dumpType),
    _intfAddWatch(std::make_unique<sdbusplus::bus::match_t>(
        bus,
        sdbusplus::bus::match::rules::interfacesAdded() + sender(dumpService),
        [this](auto& msg) { this->interfaceAdded(msg); })),
    _intfRemWatch(std::make_unique<sdbusplus::bus::match_t>(
        bus,
        sdbusplus::bus::match::rules::interfacesRemoved() + sender(dumpService),
        [this](auto& msg) { this->interfaceRemoved(msg); }))
{
}

void DBusDumpWatcher::interfaceAdded(sdbusplus::message::message& msg)
{
    sdbusplus::message::object_path objPath;
    DBusInteracesMap interfaces;
    msg.read(objPath, interfaces);
    std::string strObjPath = objPath;
    log<level::INFO>(
        fmt::format("DBusDumpWatcher::interfaceAdded path ({})", strObjPath)
            .c_str());
    auto iter = interfaces.find(_entryIntf);
    if (iter == interfaces.end())
    {
        // ignore not specific to the dump type being watched
        return;
    }
    uint32_t id = std::stoul(objPath.filename());
    _entryPropWatchList.emplace(
        strObjPath, std::make_unique<sdbusplus::bus::match_t>(
                        _bus,
                        sdbusplus::bus::match::rules::propertiesChanged(
                            strObjPath, progressIntf),
                        [this, strObjPath, id](auto& msg) {
                            this->propertiesChanged(strObjPath, id, msg);
                        }));
}

void DBusDumpWatcher::interfaceRemoved(sdbusplus::message::message& msg)
{
    sdbusplus::message::object_path objPath;
    DBusInteracesMap interfaces;
    msg.read(objPath, interfaces);
    std::string strObjPath = objPath;
    log<level::INFO>(
        fmt::format("DBusDumpWatcher::interfaceRemoved path ({})", strObjPath)
            .c_str());
    auto iter = interfaces.find(_entryIntf);
    if (iter == interfaces.end())
    {
        // ignore not specific to the dump type being watched
        return;
    }
    _entryPropWatchList.erase(strObjPath);
}

void DBusDumpWatcher::propertiesChanged(const std::string& objPath, uint32_t id,
                                        sdbusplus::message::message& msg)
{
    std::string interface;
    DBusPropertiesMap propMap;
    msg.read(interface, propMap);
    log<level::INFO>(
        fmt::format("propertiesChanged objectpath ({}) id ({})", objPath, id)
            .c_str());

    bool fcomplete = isDumpProgressCompleted(propMap);
    if (!fcomplete)
    {
        log<level::DEBUG>(
            fmt::format("propertiesChanged objectpath ({}) status is not "
                        "complete",
                        objPath)
                .c_str());
        return;
    }

    uint64_t size = getDumpSize(_bus, objPath);
    openpower::dump::pldm::sendNewDumpCmd(id, _dumpType, size);

    _entryPropWatchList.erase(objPath);
}

void DBusDumpWatcher::addInProgressDumpsToWatch(ManagedObjectType&& objects)
{
    for (auto& object : objects)
    {
        auto iter = object.second.find(_entryIntf);
        if (iter == object.second.end())
        {
            continue;
        }
        bool fcomplete = isDumpProgressCompleted(object.second);
        if (!fcomplete)
        {
            std::string objectPath = object.first;
            uint32_t id = std::stoul(object.first.filename());
            log<level::INFO>(
                fmt::format("DumpOffloadBmc addInProgresDumps to watch "
                            "objectPath ({})",
                            objectPath)
                    .c_str());
            _entryPropWatchList.emplace(
                objectPath, std::make_unique<sdbusplus::bus::match_t>(
                                _bus,
                                sdbusplus::bus::match::rules::propertiesChanged(
                                    objectPath, progressIntf),
                                [this, objectPath, id](auto& msg) {
                                    this->propertiesChanged(objectPath, id,
                                                            msg);
                                }));
        }
    }
}

} // namespace openpower::dump
