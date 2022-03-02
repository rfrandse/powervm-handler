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
}

void DumpOffloadQueue::enqueueForOffloading(const object_path& path,
                                            DumpType type)
{
    log<level::INFO>(
        fmt::format("Queue enqueue dump for offload path ({}) ", path.str)
            .c_str());
    _offloadDumpList.emplace(path.str, type);

    offload();
}

void DumpOffloadQueue::offload()
{
    try
    {
        if (_offloadObjPath.empty() && !_offloadDumpList.empty())
        {
            // do not offload if host is not running
            if (!isHostRunning(_bus))
            {
                return;
            }

            // do not offload if system is hmc managed
            if (isSystemHMCManaged(_bus))
            {
                return;
            }
            auto iter = _offloadDumpList.begin();
            _offloadObjPath = iter->first;
            object_path path = _offloadObjPath;
            uint32_t id = std::stoul(path.filename());
            uint64_t size = getDumpSize(_bus, _offloadObjPath);
            DumpType type = iter->second;
            log<level::INFO>(
                fmt::format("Queue offload initiating offload ({}) id ({}) "
                            "type ({}) size ({})",
                            _offloadObjPath, id, type, size)
                    .c_str());
            openpower::dump::pldm::sendNewDumpCmd(id, type, size);
        }
        else if (_offloadDumpList.empty())
        {
            log<level::INFO>(
                fmt::format("Queue nothing to offload listsize ({})",
                            _offloadDumpList.size())
                    .c_str());
        }
    }
    catch (const std::exception& ex)
    {
        log<level::INFO>(
            fmt::format("Queue dump ({}) deleted/pldm error ({}), try another"
                        " dump",
                        _offloadObjPath, ex.what())
                .c_str());
        // race-condition while dumps are getting deleted it tries to offload
        // but the object is not found, so try another dump rather than
        // getting stuck. PLDM could also hit race condition when it is
        // trying to get the path from D-bus it might not find the dump.

        // This race condition will hit when we deleteall the dumps while
        // one is in progres
        _offloadDumpList.erase(_offloadObjPath);
        _offloadObjPath.clear();
        offload();
    }
}

void DumpOffloadQueue::dequeueForOffloading(const object_path& path)
{
    try
    {
        if (_offloadObjPath == path) // succesfully offloaded
        {
            log<level::INFO>(
                fmt::format("Queue dequeue dump offload completed ({}) "
                            "removing from queue",
                            path.str)
                    .c_str());
            _offloadDumpList.erase(path);
            _offloadObjPath.clear();
            offload(); // offload the next dump if any
        }
        else // some other dump queued got deleted so simply remove from queue
        {
            log<level::INFO>(
                fmt::format("Queue dequeue offload not started dump "
                            "deleted ({}) removing from queue",
                            path.str)
                    .c_str());
            _offloadDumpList.erase(path);
        }
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Queue exception in dequeue ({})", ex.what()).c_str());
        throw;
    }
}
} // namespace openpower::dump
