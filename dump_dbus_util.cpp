#include "config.h"

#include "dump_dbus_util.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>
#include <variant>

namespace openpower::dump
{
using ::phosphor::logging::level;
using ::phosphor::logging::log;

bool isDumpProgressCompleted(const DBusInteracesMap& intfMap)
{
    for (auto& intf : intfMap)
    {
        if (intf.first == progressIntf)
        {
            for (auto& prop : intf.second)
            {
                if (prop.first == "Status")
                {
                    auto status = std::get_if<std::string>(&prop.second);
                    if (status != nullptr)
                    {
                        if (*status == progressComplete)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool isDumpProgressCompleted(const DBusPropertiesMap& propMap)
{
    for (auto prop : propMap)
    {
        if (prop.first == "Status")
        {
            auto status = std::get_if<std::string>(&prop.second);
            if (status != nullptr)
            {
                if (*status == progressComplete)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

uint64_t getDumpSize(sdbusplus::bus::bus& bus, const std::string& objectPath)
{
    uint64_t size = 0;
    try
    {
        openpower::dump::utility::DbusVariantType value;
        auto method = bus.new_method_call(dumpService, objectPath.c_str(),
                                          dbusPropIntf, "Get");
        method.append(entryIntf);
        method.append("Size");
        auto reply = bus.call(method);
        reply.read(value);
        const uint64_t* sizePtr = std::get_if<uint64_t>(&value);
        if (sizePtr)
        {
            size = *sizePtr;
        }
        else
        {
            std::string err = fmt::format(
                "Size value not set for dump object ({})", objectPath);
            log<level::ERR>(err.c_str());
            throw std::runtime_error(err);
        }
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Failed to get dump size property ({})", ex.what())
                .c_str());
        throw;
    }
    return size;
}

ManagedObjectType getDumpEntries(sdbusplus::bus::bus& bus)
{
    ManagedObjectType objects;
    try
    {
        auto getManagedObjects = bus.new_method_call(
            dumpService, dumpObjPath, dbusObjManagerIntf, "GetManagedObjects");
        auto reply = bus.call(getManagedObjects);
        reply.read(objects);
    }
    catch (const sdbusplus::exception::exception& ex)
    {
        log<level::ERR>(
            fmt::format("Failed to get dump entries list ({})", ex.what())
                .c_str());
        throw;
    }
    return objects;
}
} // namespace openpower::dump
