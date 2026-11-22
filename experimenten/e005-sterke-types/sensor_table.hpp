// e005 — sensor_table: a fixed table with a strong handle (P2 rules 6, 7).
//
// Nothing is ever removed from a Sensor_table, so a Sensor_id stays valid for
// the life of the table. The table is the only code that turns a number into
// a Sensor_id: sensor_add for its own entries, sensor_find for a number that
// comes from outside.
#pragma once

#include "units.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>

namespace e005 {

struct Sensor_id {
    explicit constexpr Sensor_id(std::uint16_t v) : value{v} {}
    std::uint16_t value;
    friend constexpr bool operator==(Sensor_id, Sensor_id) = default;
};

// A second handle type, so that the tests can show that the two do not mix.
struct Actuator_id {
    explicit constexpr Actuator_id(std::uint16_t v) : value{v} {}
    std::uint16_t value;
    friend constexpr bool operator==(Actuator_id, Actuator_id) = default;
};

struct Sensor {
    Millivolt reading{0};
    std::uint8_t channel = 0;
};

inline constexpr std::size_t max_sensors = 8;

struct Sensor_table {
    std::array<Sensor, max_sensors> sensors{};
    std::uint16_t used = 0;
};
static_assert(sizeof(Sensor_table) <= 72, "Sensor_table grew; check the memory budget");

enum class Table_error : std::uint8_t {
    full,
    unknown_id,
};

// Adds sensor. Returns its handle, or Table_error::full.
std::expected<Sensor_id, Table_error> sensor_add(Sensor_table &table, Sensor sensor);

// Returns the sensor for an id from our own code. A bad id is a programming
// error: assert.
const Sensor &sensor_at(const Sensor_table &table, Sensor_id id);

// Turns a number from outside (a message, a configuration file) into a handle.
// Returns Table_error::unknown_id when no sensor has that number.
std::expected<Sensor_id, Table_error> sensor_find(const Sensor_table &table, std::uint16_t raw);

}  // namespace e005
