#include "sensor_pool.hpp"

#include <cassert>
#include <cstddef>

namespace e005 {

namespace {

// True when handle names a sensor that is in the pool now.
bool s_is_current(const Sensor_pool &pool, Sensor_handle handle)
{
    if (handle.index >= pool.slots.size()) {
        return false;
    }
    const Pool_slot &slot = pool.slots.at(handle.index);
    return slot.used && slot.generation == handle.generation;
}

}  // namespace

std::expected<Sensor_handle, Pool_error> sensor_pool_add(Sensor_pool &pool, Sensor sensor)
{
    for (std::size_t i = 0; i < pool.slots.size(); ++i) {
        Pool_slot &slot = pool.slots.at(i);
        if (!slot.used) {
            slot.sensor = sensor;
            slot.used = true;
            return Sensor_handle{static_cast<std::uint16_t>(i), slot.generation};
        }
    }
    return std::unexpected{Pool_error::full};
}

std::expected<void, Pool_error> sensor_pool_remove(Sensor_pool &pool, Sensor_handle handle)
{
    if (!s_is_current(pool, handle)) {
        return std::unexpected{Pool_error::stale_handle};
    }
    Pool_slot &slot = pool.slots.at(handle.index);
    slot.used = false;
    // Wraps from 65535 to 0 on purpose; see the limit in sensor_pool.hpp.
    slot.generation = static_cast<std::uint16_t>(slot.generation + 1U);
    return {};
}

const Sensor &sensor_pool_at(const Sensor_pool &pool, Sensor_handle handle)
{
    assert(s_is_current(pool, handle));
    return pool.slots.at(handle.index).sensor;
}

std::expected<Sensor, Pool_error> sensor_pool_find(const Sensor_pool &pool, Sensor_handle handle)
{
    if (!s_is_current(pool, handle)) {
        return std::unexpected{Pool_error::stale_handle};
    }
    return pool.slots.at(handle.index).sensor;
}

}  // namespace e005
