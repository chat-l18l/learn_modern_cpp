// e005 — tests for the strong types.
//
// Part 1 runs at compile time. Each static_assert states a mistake that the
// type must reject. If a later change weakens a type, this file stops
// compiling. A requires-expression only yields false inside a template, so
// each property is a concept with the type as parameter.
//
// Part 2 runs at run time: the tables, the generation check and its limit.
#include "sensor_pool.hpp"
#include "sensor_table.hpp"
#include "units.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace {

using e005::Actuator_id;
using e005::Deci_celsius;
using e005::Millivolt;
using e005::Sensor_handle;
using e005::Sensor_id;
using e005::Sensor_table;

// --- Part 1: compile time ----------------------------------------------

template <typename T>
concept Adds_int = requires(T a) { a + 1; };
template <typename T>
concept Orders = requires(T a) { a < a; };
template <typename T>
concept Compares = requires(T a) { a == a; };
template <typename A, typename B>
concept Compares_with = requires(A a, B b) { a == b; };
template <typename R>
concept Sensor_at_takes_braced = requires(R raw) { e005::sensor_at(Sensor_table{}, {raw}); };

// Handles: a name for an element, not a number.
static_assert(!std::is_convertible_v<std::uint16_t, Sensor_id>);
static_assert(!std::is_convertible_v<std::size_t, Sensor_id>);
static_assert(!std::is_convertible_v<Sensor_id, std::uint16_t>);
static_assert(!Adds_int<Sensor_id>);
static_assert(!Orders<Sensor_id>);
static_assert(Compares<Sensor_id>);
static_assert(!Compares_with<Sensor_id, Actuator_id>);
static_assert(!std::is_invocable_v<decltype(e005::sensor_at), const Sensor_table &, Actuator_id>);
static_assert(!std::is_invocable_v<decltype(e005::sensor_at), const Sensor_table &, int>);
static_assert(!Sensor_at_takes_braced<std::uint16_t>);
static_assert(std::is_trivially_copyable_v<Sensor_id>);
static_assert(sizeof(Sensor_id) == sizeof(std::uint16_t));

// A generation handle is still a name: no arithmetic, no order.
static_assert(!Adds_int<Sensor_handle>);
static_assert(!Orders<Sensor_handle>);
static_assert(!Compares_with<Sensor_handle, Sensor_id>);
static_assert(sizeof(Sensor_handle) == 2 * sizeof(std::uint16_t));

// Quantities: comparable within one unit, never across units.
static_assert(!std::is_convertible_v<std::int32_t, Millivolt>);
static_assert(!std::is_convertible_v<Deci_celsius, Millivolt>);
static_assert(Orders<Millivolt>);
static_assert(!Compares_with<Millivolt, Deci_celsius>);
static_assert(!Adds_int<Millivolt>);
static_assert(!std::is_invocable_v<decltype(e005::limits_contain), e005::Limits, Deci_celsius>);
static_assert(!std::is_invocable_v<decltype(e005::limits_contain), e005::Limits, int>);
static_assert(!std::is_default_constructible_v<Millivolt>);  // a Limits field cannot be skipped

// --- Part 2: run time ---------------------------------------------------

TEST(SensorTable, AddReturnsHandlesInOrder)
{
    Sensor_table table;
    const auto a = e005::sensor_add(table, {.reading = Millivolt{100}, .channel = 1});
    const auto b = e005::sensor_add(table, {.reading = Millivolt{200}, .channel = 2});
    ASSERT_TRUE(a.has_value());
    ASSERT_TRUE(b.has_value());
    EXPECT_EQ(e005::sensor_at(table, *a).channel, 1);
    EXPECT_EQ(e005::sensor_at(table, *b).channel, 2);
}

TEST(SensorTable, RefusesWhenFull)
{
    Sensor_table table;
    for (std::size_t i = 0; i < e005::max_sensors; ++i) {
        ASSERT_TRUE(e005::sensor_add(table, {}).has_value());
    }
    EXPECT_EQ(e005::sensor_add(table, {}).error(), e005::Table_error::full);
}

TEST(SensorTable, FindTurnsAKnownNumberIntoAHandle)
{
    Sensor_table table;
    ASSERT_TRUE(e005::sensor_add(table, {.reading = Millivolt{5}, .channel = 7}).has_value());
    const auto id = e005::sensor_find(table, 0);
    ASSERT_TRUE(id.has_value());
    EXPECT_EQ(e005::sensor_at(table, *id).channel, 7);
}

TEST(SensorTable, FindRefusesAnUnknownNumber)
{
    Sensor_table table;
    ASSERT_TRUE(e005::sensor_add(table, {}).has_value());
    EXPECT_EQ(e005::sensor_find(table, 1).error(), e005::Table_error::unknown_id);
    EXPECT_EQ(e005::sensor_find(table, 65535).error(), e005::Table_error::unknown_id);
}

TEST(SensorPool, OldHandleIsRefusedAfterTheSlotIsReused)
{
    // G28: the plain-index demo reads the wrong sensor here.
    e005::Sensor_pool pool;
    const auto old_handle = e005::sensor_pool_add(pool, {.reading = Millivolt{1}, .channel = 1});
    ASSERT_TRUE(old_handle.has_value());
    ASSERT_TRUE(e005::sensor_pool_remove(pool, *old_handle).has_value());
    const auto new_handle = e005::sensor_pool_add(pool, {.reading = Millivolt{2}, .channel = 2});
    ASSERT_TRUE(new_handle.has_value());

    EXPECT_EQ(new_handle->index, old_handle->index);  // same slot ...
    EXPECT_NE(*new_handle, *old_handle);              // ... different handle
    EXPECT_EQ(e005::sensor_pool_find(pool, *old_handle).error(), e005::Pool_error::stale_handle);
    EXPECT_EQ(e005::sensor_pool_find(pool, *new_handle)->channel, 2);
}

TEST(SensorPool, RemovingTwiceIsRefused)
{
    e005::Sensor_pool pool;
    const auto handle = e005::sensor_pool_add(pool, {});
    ASSERT_TRUE(handle.has_value());
    ASSERT_TRUE(e005::sensor_pool_remove(pool, *handle).has_value());
    EXPECT_EQ(e005::sensor_pool_remove(pool, *handle).error(), e005::Pool_error::stale_handle);
}

TEST(SensorPool, GenerationWrapsAfter65536Reuses)
{
    // The limit of a 16-bit generation, measured: after 65536 remove/add
    // cycles on one slot, the very first handle matches again.
    e005::Sensor_pool pool;
    const auto first = e005::sensor_pool_add(pool, {.reading = Millivolt{1}, .channel = 1});
    ASSERT_TRUE(first.has_value());
    auto current = *first;
    for (std::uint32_t i = 0; i < 65536U; ++i) {
        ASSERT_TRUE(e005::sensor_pool_remove(pool, current).has_value());
        const auto next = e005::sensor_pool_add(pool, {.reading = Millivolt{2}, .channel = 2});
        ASSERT_TRUE(next.has_value());
        current = *next;
    }
    EXPECT_EQ(current, *first);                                   // the generation wrapped
    EXPECT_EQ(e005::sensor_pool_find(pool, *first)->channel, 2);  // old handle, new sensor
}

TEST(Limits, AcceptsLowBelowHigh)
{
    const auto limits = e005::limits_check({.low = Millivolt{100}, .high = Millivolt{900}});
    ASSERT_TRUE(limits.has_value());
    EXPECT_TRUE(e005::limits_contain(*limits, Millivolt{500}));
    EXPECT_FALSE(e005::limits_contain(*limits, Millivolt{901}));
}

TEST(Limits, RefusesLowAboveHigh)
{
    // The swap that a type cannot see: both fields are Millivolt.
    const auto limits = e005::limits_check({.low = Millivolt{900}, .high = Millivolt{100}});
    EXPECT_EQ(limits.error(), e005::Limits_error::low_above_high);
}

}  // namespace
