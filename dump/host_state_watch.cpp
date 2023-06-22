#include "host_state_watch.hpp"

#include "dbus_util.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DBusPropertiesMap;
using ::phosphor::logging::level;
using ::phosphor::logging::log;

HostStateWatch::HostStateWatch(sdbusplus::bus::bus& bus,
                               HostOffloaderQueue& dumpQueue) :
    _bus(bus),
    _dumpQueue(dumpQueue)
{
    _hostStatePropWatch = std::make_unique<sdbusplus::bus::match_t>(
        _bus,
        sdbusplus::bus::match::rules::propertiesChanged(
            "/xyz/openbmc_project/state/host0",
            "xyz.openbmc_project.State.Boot.Progress"),
        [this](auto& msg) { this->propertyChanged(msg); });
}

void HostStateWatch::propertyChanged(sdbusplus::message::message& msg)
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
                if (bootProgress == ProgressStages::OSRunning)
                {
                    log<level::INFO>("Host state changed to OSRunning");
                    _dumpQueue.hostStateChange(true);
                }
                else
                {
                    log<level::INFO>("Host state changed to not running");
                    _dumpQueue.hostStateChange(false);
                }
            }
            else
            {
                log<level::INFO>("Host state changed to not running");
                _dumpQueue.hostStateChange(false);
            }
        }
    }
}

} // namespace openpower::dump
