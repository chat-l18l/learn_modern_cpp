// e005 — units: physical quantities as strong types (P2 rule 10).
//
// A quantity has the unit and the resolution in its name. It has an explicit
// constructor, one member `value`, and only the operations the quantity
// needs. Here that is comparison: a limit is checked against a reading.
// There is no arithmetic on purpose: adding two int32 values can overflow,
// and that would need its own contract.
#pragma once

#include <compare>
#include <cstdint>
#include <expected>

namespace e005 {

struct Millivolt {
    explicit constexpr Millivolt(std::int32_t v) : value{v} {}
    std::int32_t value;
    friend constexpr bool operator==(Millivolt, Millivolt) = default;
    friend constexpr auto operator<=>(Millivolt, Millivolt) = default;
};

struct Deci_celsius {
    explicit constexpr Deci_celsius(std::int16_t v) : value{v} {}
    std::int16_t value;
    friend constexpr bool operator==(Deci_celsius, Deci_celsius) = default;
    friend constexpr auto operator<=>(Deci_celsius, Deci_celsius) = default;
};

// An alarm band. The fields have names, so a caller writes
//   Limits{.low = Millivolt{100}, .high = Millivolt{900}}
// and C++ rejects the designators in the wrong order (see compile_fail/).
// Millivolt has no default constructor, so a caller cannot leave one out.
struct Limits {
    Millivolt low;
    Millivolt high;
};

enum class Limits_error : std::uint8_t {
    low_above_high,
};

// Returns limits unchanged when low <= high. Limits come from configuration,
// that is from outside, so a bad pair is an error value, not an assert.
std::expected<Limits, Limits_error> limits_check(Limits limits);

// True when reading lies inside limits, bounds included.
bool limits_contain(Limits limits, Millivolt reading);

}  // namespace e005
