#include "config.h"

#include "dump_dbus_watch.hpp"

#include "dump_dbus_util.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DBusInteracesList;
using ::openpower::dump::utility::DBusInteracesMap;
using ::openpower::dump::utility::DBusPropertiesMap;
using ::openpower::dump::utility::ManagedObjectType;
using ::phosphor::logging::level;
using ::phosphor::logging::log;
using ::sdbusplus::bus::match::rules::sender;

DumpDBusWatch::DumpDBusWatch(sdbusplus::bus::bus& bus,
                             DumpOffloadQueue& dumpOffloader,
                             const std::string& entryIntf, DumpType dumpType) :
    _bus(bus),
    _dumpOffloader(dumpOffloader), _entryIntf(entryIntf), _dumpType(dumpType)
{
    // we could watch only on the "sender" for the intefacesAdded signal rather
    // than on the entry interface, we will be notified for all the new dumps,
    // but pick only the ones we are interested.
    _intfAddWatch = std::make_unique<sdbusplus::bus::match_t>(
        bus,
        sdbusplus::bus::match::rules::interfacesAdded() + sender(dumpService),
        [this](auto& msg) { this->interfaceAdded(msg); });

    _intfRemWatch = std::make_unique<sdbusplus::bus::match_t>(
        bus,
        sdbusplus::bus::match::rules::interfacesRemoved() + sender(dumpService),
        [this](auto& msg) { this->interfaceRemoved(msg); });
}

void DumpDBusWatch::interfaceAdded(sdbusplus::message::message& msg)
{
    try
    {
        // system can change from non-hmc to hmc managed system, do not offload
        // if system changed to hmc managed system
        if (isSystemHMCManaged(_bus))
        {
            return;
        }
        // Do not offload if host is not running
        if (!isHostRunning(_bus))
        {
            return;
        }
        sdbusplus::message::object_path objPath;
        DBusInteracesMap interfaces;
        msg.read(objPath, interfaces);
        auto iter = interfaces.find(_entryIntf);
        if (iter == interfaces.end())
        {
            // ignore not specific to the dump type being watched
            return;
        }
        log<level::INFO>(
            fmt::format("interfaceAdded path ({})", objPath.str).c_str());
        uint32_t id = std::stoul(objPath.filename());
        _entryPropWatchList.emplace(
            objPath, std::make_unique<sdbusplus::bus::match_t>(
                         _bus,
                         sdbusplus::bus::match::rules::propertiesChanged(
                             objPath, progressIntf),
                         [this, objPath, id](auto& msg) {
                             this->propertiesChanged(objPath, id, msg);
                         }));
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Exception in watch interfaceAdded ({})", ex.what())
                .c_str());
        throw;
    }
}

void DumpDBusWatch::interfaceRemoved(sdbusplus::message::message& msg)
{
    try
    {
        sdbusplus::message::object_path objPath;
        DBusInteracesList interfaces;
        msg.read(objPath, interfaces);
        auto iter = std::find(interfaces.begin(), interfaces.end(), _entryIntf);
        if (iter == interfaces.end())
        {
            // ignore not specific to the dump type being watched
            return;
        }
        log<level::INFO>(
            fmt::format("interfaceRemoved path ({})", objPath.str).c_str());

        // Remove from watch if incase the dump gets deleted while waiting for
        // completion
        _entryPropWatchList.erase(objPath);
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Exception in watch interfaceRemoved ({})", ex.what())
                .c_str());
        throw;
    }
}

void DumpDBusWatch::propertiesChanged(const object_path& objPath, uint32_t id,
                                      sdbusplus::message::message& msg)
{
    try
    {
        // system can change from non-hmc to hmc managed system, do not offload
        // if system changed to hmc managed system
        if (isSystemHMCManaged(_bus))
        {
            return;
        }
        // Do not offload if host is not running
        if (!isHostRunning(_bus))
        {
            return;
        }
        std::string interface;
        DBusPropertiesMap propMap;
        msg.read(interface, propMap);
        log<level::INFO>(
            fmt::format("propertiesChanged object path ({}) id ({})",
                        objPath.str, id)
                .c_str());

        bool fcomplete = isDumpProgressCompleted(propMap);
        if (!fcomplete)
        {
            log<level::DEBUG>(
                fmt::format("propertiesChanged object path ({}) status is not "
                            "complete",
                            objPath.str)
                    .c_str());
            return;
        }

        // queue the dump for offloading
        _dumpOffloader.enqueueForOffloading(objPath, _dumpType);

        _entryPropWatchList.erase(objPath);
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Exception in watch propertiesChanged ({})", ex.what())
                .c_str());
        throw;
    }
}

void DumpDBusWatch::addInProgressDumpsToWatch(const ManagedObjectType& objects)
{
    try
    {
        for (auto& object : objects)
        {
            object_path objectPath = object.first;
            auto iter = object.second.find(_entryIntf);
            if (iter == object.second.end())
            {
                continue;
            }
            bool fcomplete = isDumpProgressCompleted(object.second);
            if (!fcomplete)
            {
                uint32_t id = std::stoul(object.first.filename());
                log<level::INFO>(
                    fmt::format("addInProgressDumpsToWatch object path ({})",
                                objectPath.str)
                        .c_str());
                _entryPropWatchList.emplace(
                    objectPath,
                    std::make_unique<sdbusplus::bus::match_t>(
                        _bus,
                        sdbusplus::bus::match::rules::propertiesChanged(
                            objectPath, progressIntf),
                        [this, objectPath, id](auto& msg) {
                            this->propertiesChanged(objectPath, id, msg);
                        }));
            }
        }
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Exception in addInProgressDumpsToWatch ({})",
                        ex.what())
                .c_str());
        throw;
    }
}

} // namespace openpower::dump
