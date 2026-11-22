// e006 — sensor_list: a small value type, and a function that borrows from it.
//
// readings_of returns a reference INTO its argument. The caller must keep the
// argument alive while it uses the reference. E006_LIFETIMEBOUND tells Clang
// about that borrow, so that Clang can warn when the argument is a temporary.
#pragma once

#include <array>
#include <optional>

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(clang::lifetimebound)
#define E006_LIFETIMEBOUND [[clang::lifetimebound]]
#endif
#endif
#ifndef E006_LIFETIMEBOUND
#define E006_LIFETIMEBOUND
#endif

namespace e006 {

using Readings = std::array<int, 4>;

// Plain data (P1): the readings are a public member.
struct Sensor_list {
    Readings readings{};
};

// Returns a list with the readings 1, 2, 3, 4.
Sensor_list make_sensor_list();

// Returns that list when present is true, and nothing otherwise.
std::optional<Sensor_list> make_optional_list(bool present);

// Returns a reference to the readings of list. Borrows from list.
const Readings &readings_of(const Sensor_list &list E006_LIFETIMEBOUND);

// Returns the sum of the readings. A pure function: it keeps no reference.
int sum_readings(const Readings &readings);

}  // namespace e006
