#pragma once

#include <sdbusplus/message.hpp>
#include <sdbusplus/utility/dedup_variant.hpp>

namespace openpower::dump::utility
{
// clang-format off
using DbusVariantType = sdbusplus::utility::dedup_variant_t<
    std::string,
    int64_t,
    uint64_t,
    double,
    int32_t,
    uint32_t,
    int16_t,
    uint16_t,
    uint8_t,
    bool,
    size_t
 >;

// clang-format on
using DBusPropertiesMap = std::map<std::string, DbusVariantType>;
using DBusInteracesMap = std::map<std::string, DBusPropertiesMap>;
using ManagedObjectType =
    std::vector<std::pair<sdbusplus::message::object_path, DBusInteracesMap>>;

/**
 * @brief Type of the dumps
 */
enum class DumpType
{
    bmc,
    hardware,
    hostboot,
    sbe
};

} // namespace openpower::dump::utility
