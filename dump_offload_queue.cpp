#include "config.h"

#include "dump_offload_queue.hpp"

#include "dump_dbus_util.hpp"
#include "dump_send_pldm_cmd.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DBusInteracesList;
using ::phosphor::logging::level;
using ::phosphor::logging::log;
using ::sdbusplus::bus::match::rules::sender;

DumpOffloadQueue::DumpOffloadQueue(sdbusplus::bus::bus& bus) : _bus(bus)
{
    _intfRemWatch = std::make_unique<sdbusplus::bus::match_t>(
        bus,
        sdbusplus::bus::match::rules::interfacesRemoved() + sender(dumpService),
        [this](auto& msg) { this->interfaceRemoved(msg); });
}

void DumpOffloadQueue::enqueueForOffloading(const object_path& path,
                                            DumpType type)
{
    log<level::INFO>(
        fmt::format("queueing for offload path ({}) ", path.str).c_str());
    _offloadDumpList.emplace(path.str, type);
    offload();
}

void DumpOffloadQueue::offload()
{
    try
    {
        if (_offloadObjPath.empty() && !_offloadDumpList.empty())
        {
            auto iter = _offloadDumpList.begin();
            _offloadObjPath = iter->first;
            object_path path = _offloadObjPath;
            uint32_t id = std::stoul(path.filename());
            uint64_t size = getDumpSize(_bus, _offloadObjPath);
            DumpType type = iter->second;
            log<level::INFO>(
                fmt::format("offload queue initiating offload ({}) id ({}) "
                            "type ({}) size ({})",
                            _offloadObjPath, id, type, size)
                    .c_str());
            openpower::dump::pldm::sendNewDumpCmd(id, type, size);
        }
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("exception caught to send pldm cmd ({})", ex.what())
                .c_str());
        throw;
    }
}

void DumpOffloadQueue::interfaceRemoved(sdbusplus::message::message& msg)
{
    try
    {
        sdbusplus::message::object_path objPath;
        DBusInteracesList interfaces;
        msg.read(objPath, interfaces);

        if (_offloadObjPath == objPath.str)
        {
            log<level::INFO>(
                fmt::format("offload completed ({}) removing from queue",
                            _offloadObjPath)
                    .c_str());
            _offloadDumpList.erase(objPath.str);
            _offloadObjPath.clear();
            offload(); // offload the next dump if any
        }
        else
        {
            // delete from the offload list if incase the dump get deleted
            // while it is queued for offload
            _offloadDumpList.erase(objPath.str);
        }
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("exception in offloadqueue interfaceRemoved ({})",
                        ex.what())
                .c_str());
        throw;
    }
}
} // namespace openpower::dump
