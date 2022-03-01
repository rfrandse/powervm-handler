#include "dump_offload_handler.hpp"

#include "dump_dbus_util.hpp"
#include "dump_utility.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>

namespace openpower::dump
{
using ::openpower::dump::utility::DumpType;
using ::openpower::dump::utility::ManagedObjectType;
using ::phosphor::logging::level;
using ::phosphor::logging::log;

DumpOffloadHandler::DumpOffloadHandler(sdbusplus::bus::bus& bus,
                                       DumpOffloadQueue& dumpOffloader,
                                       const std::string& entryIntf,
                                       DumpType dumpType) :
    _bus(bus),
    _dumpOffloader(dumpOffloader), _entryIntf(entryIntf), _dumpType(dumpType),
    _dumpWatch(bus, dumpOffloader, entryIntf, dumpType)
{
}

void DumpOffloadHandler::offload(const ManagedObjectType& objects)
{
    try
    {
        ManagedObjectType inProgressDumps;
        for (auto& object : objects)
        {
            auto iter = object.second.find(_entryIntf);
            if (iter == object.second.end())
            {
                continue; // not watching
            }
            bool fcomplete = isDumpProgressCompleted(object.second);
            if (!fcomplete)
            {
                log<level::INFO>(fmt::format("Dump generation is not completed "
                                             "so not offloading ({})",
                                             object.first.str)
                                     .c_str());
                inProgressDumps.emplace_back(object.first, object.second);
                continue;
            }
            log<level::INFO>(
                fmt::format("DumpOffloadHandler::offload dump object ({})",
                            object.first.str)
                    .c_str());
            // queue the dump for offloading
            _dumpOffloader.enqueueForOffloading(object.first, _dumpType);

        } // end for

        // add any inprogress dumps to the watch list
        _dumpWatch.addInProgressDumpsToWatch(inProgressDumps);
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Failed to offload dump({}) ({})", _dumpType, ex.what())
                .c_str());
        throw;
    }
}

} // namespace openpower::dump
