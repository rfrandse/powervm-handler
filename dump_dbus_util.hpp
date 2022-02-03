#pragma once

#include "dump_utility.hpp"

#include <cstdint>

namespace openpower::dump
{
using ::openpower::dump::utility::DBusInteracesMap;
using ::openpower::dump::utility::DBusPropertiesMap;
using ::openpower::dump::utility::ManagedObjectType;
/**
 * @brief Read progress property from the interface map object
 * @param[in] intfMap map of interfaces and its properties
 * @return true if progress is complete else false
 */
bool isDumpProgressCompleted(const DBusInteracesMap& intfMap);

/**
 * @brief Read progress property from the interface map object
 * @param[in] propMao map of properties and its values
 * @return true if progress is complete else false
 */
bool isDumpProgressCompleted(const DBusPropertiesMap& propMap);

/**
 * @brief Read dump size from the interface map object
 * @param[in] bus - D-Bus handle
 * @param[in] objectPath - path of the D-Bus entry object
 * @return dump size value
 */
uint64_t getDumpSize(sdbusplus::bus::bus& bus, const std::string& objectPath);

/**
 * @brief Read available dump entries from the system
 * @param[in] bus D-Bus handle
 * @return D-Bus entries with properties
 */
ManagedObjectType getDumpEntries(sdbusplus::bus::bus& bus);
} // namespace openpower::dump
