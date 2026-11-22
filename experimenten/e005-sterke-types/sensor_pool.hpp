// e005 — sensor_pool: a table that removes, with generation-checked handles
// (P2 rule 8).
//
// A removed slot is used again by the next add. A plain index to that slot
// would then name the new sensor (catalogue G28). A Sensor_handle carries the
// generation of its slot; remove increments the generation, so every old
// handle stops matching.
//
// Limit: the generation has 16 bits. After 65536 reuses of one slot the
// generation wraps, and a handle that old matches again. The tests measure
// this. A design that must rule it out retires a slot when its generation
// would wrap.
#pragma once

#include "sensor_table.hpp"

#include <array>
#include <cstdint>
#include <expected>

namespace e005 {

// clang-tidy reports bugprone-easily-swappable-parameters on this constructor:
// index and generation are both std::uint16_t, so Sensor_handle{gen, index}
// compiles. That is catalogue G29 in our own code. Mitigation: only
// sensor_pool_add constructs a Sensor_handle, and the tests cover it.
struct Sensor_handle {
    explicit constexpr Sensor_handle(std::uint16_t index_, std::uint16_t generation_)
        : index{index_}, generation{generation_}
    {}
    std::uint16_t index;
    std::uint16_t generation;
    friend constexpr bool operator==(Sensor_handle, Sensor_handle) = default;
};

struct Pool_slot {
    Sensor sensor{};
    std::uint16_t generation = 0;
    bool used = false;
};

struct Sensor_pool {
    std::array<Pool_slot, max_sensors> slots{};
};
static_assert(sizeof(Sensor_pool) <= 128, "Sensor_pool grew; check the memory budget");

enum class Pool_error : std::uint8_t {
    full,
    stale_handle,
};

// Adds sensor in the first free slot. Returns its handle, or Pool_error::full.
std::expected<Sensor_handle, Pool_error> sensor_pool_add(Sensor_pool &pool, Sensor sensor);

// Removes the sensor of handle. Returns Pool_error::stale_handle when the
// handle no longer names a sensor.
std::expected<void, Pool_error> sensor_pool_remove(Sensor_pool &pool, Sensor_handle handle);

// Returns the sensor for a handle from our own code. A stale handle is a
// programming error: assert.
const Sensor &sensor_pool_at(const Sensor_pool &pool, Sensor_handle handle);

// Returns a copy of the sensor, or Pool_error::stale_handle. For handles that
// may be old, for example handles that came back in a message.
std::expected<Sensor, Pool_error> sensor_pool_find(const Sensor_pool &pool, Sensor_handle handle);

}  // namespace e005
