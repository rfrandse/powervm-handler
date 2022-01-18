#include <sdbusplus/bus.hpp>
#include <sdeventplus/source/event.hpp>

namespace openpower::dump
{
class DumpOffloadManager
{
  public:
    DumpOffloadManager() = delete;
    DumpOffloadManager(const DumpOffloadManager&) = delete;
    DumpOffloadManager& operator=(const DumpOffloadManager&) = delete;
    DumpOffloadManager(DumpOffloadManager&&) = delete;
    DumpOffloadManager& operator=(DumpOffloadManager&&) = delete;
    virtual ~DumpOffloadManager() = default;

    /** @brief Constructor to put object onto bus at a dbus path.
     *  @param[in] bus - Bus to attach to.
     *  @param[in] event- event handle
     */
    DumpOffloadManager(sdbusplus::bus::bus& bus, sdeventplus::Event& event) :
        bus(bus), event(event)
    {
    }

  private:
    // sdbusplus DBus bus connection.
    sdbusplus::bus::bus& bus;
    // sdevent Event handle
    sdeventplus::Event& event;
};
} // namespace openpower::dump
