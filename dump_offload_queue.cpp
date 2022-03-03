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

constexpr auto timeoutInMilliSeconds = 5000; // 5 sec

DumpOffloadQueue::DumpOffloadQueue(sdbusplus::bus::bus& bus,
                                   sdeventplus::Event& event) :
    _bus(bus),
    _event(event), _offloadTimeout(timeoutInMilliSeconds),
    _offloadTimer(event,
                  std::bind(std::mem_fn(&DumpOffloadQueue::timerExpired), this),
                  _offloadTimeout)
{
    // initally read the value as this app might run after host is started
    isHostRunning = openpower::dump::isHostRunning(_bus);
    isHMCManagedSystem = openpower::dump::isSystemHMCManaged(_bus);

    // intially stop the timer, start only when dumps are added to the queue
    stopTimer();
}

void DumpOffloadQueue::startTimer()
{
    if (!_offloadTimer.isEnabled() && isHostRunning && !isHMCManagedSystem &&
        !_offloadDumpList.empty())
    {
        log<level::INFO>(
            fmt::format("Queue start timer host running ({}) hmcmanaged ({})"
                        "Dumps size  ({})",
                        isHostRunning, isHMCManagedSystem,
                        _offloadDumpList.size())
                .c_str());
        _offloadTimer.setEnabled(true);
    }
    else if (_offloadTimer.isEnabled() && !isHostRunning)
    {
        log<level::INFO>("Queue stop timer host is not in running state");
        stopTimer();
    }
    else if (_offloadTimer.isEnabled() && isHMCManagedSystem)
    {
        log<level::INFO>("Queue stop timer system is HMC managed");
        stopTimer();
    }
}

void DumpOffloadQueue::stopTimer()
{
    log<level::INFO>(
        fmt::format("Queue stop timer host running ({}) hmcmanaged ({})"
                    "Dumps size  ({})",
                    isHostRunning, isHMCManagedSystem, _offloadDumpList.size())
            .c_str());
    _offloadTimer.setEnabled(false);
}

void DumpOffloadQueue::timerExpired()
{
    offload();
}

void DumpOffloadQueue::hostStateChange(bool isRunning)
{
    isHostRunning = isRunning;
    if (isHostRunning)
    {
        log<level::INFO>("Queue host state changed to running");
        // dumps might have been queued while host is not running, offload them
        startTimer();
    }
    else
    {
        log<level::INFO>("Queue host state changed to not running");
        stopTimer();
    }
}

void DumpOffloadQueue::hmcStateChange(bool isHMCManagedSystem)
{
    isHMCManagedSystem = isHMCManagedSystem;
    if (!isHMCManagedSystem)
    {
        log<level::INFO>("Queue HMC state change non HMC managed system");
        // dumps might have been queued while system is HMC managed, offload
        // them
        startTimer();
    }
    else
    {
        log<level::INFO>("Queue HMC state change HMC managed system");
        stopTimer();
    }
}

void DumpOffloadQueue::offload()
{
    try
    {
        if (_offloadInProgress)
        {
            // offload is in progress return
            return;
        }
        if (_offloadDumpList.empty())
        {
            // nothing to offload return
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
        _offloadInProgress = true;
    }
    catch (const std::exception& ex)
    {
        // PLDM could return error, if the current dump offloading is deleted
        // do not throw the error to the caller.
        log<level::ERR>(fmt::format("Queue dump ({}) deleted/pldm error ({})",
                                    _offloadObjPath, ex.what())
                            .c_str());

        // error, deque the dump from offloading
        dequeue(_offloadObjPath);
    }
}

void DumpOffloadQueue::enqueue(const object_path& path, DumpType type)
{
    log<level::INFO>(fmt::format("Queue enqueue dump ({}) size of Q ({})",
                                 path.str, _offloadDumpList.size())
                         .c_str());
    _offloadDumpList.emplace(path.str, type);

    // new dump ready to offload start timer, if not started
    startTimer();
}

void DumpOffloadQueue::dequeue(const object_path& path)
{
    log<level::INFO>(fmt::format("Queue dequeue ({}) size of Q ({})", path.str,
                                 _offloadDumpList.size())
                         .c_str());
    if (_offloadObjPath == path) // succesfully offloaded
    {
        log<level::INFO>(
            fmt::format("Queue offloaded dump completed ({}) ", path.str)
                .c_str());
        _offloadObjPath.clear();
        _offloadInProgress = false;
    }
    _offloadDumpList.erase(path);

    // if no more dumps to offload stop the timer
    if (_offloadDumpList.empty())
    {
        log<level::INFO>(
            fmt::format("Queue offloaded dump completed ({}) ", path.str)
                .c_str());
        stopTimer();
    }
}
} // namespace openpower::dump
