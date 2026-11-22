// e004 — counter: the transparent alternative to a capturing lambda.
//
// The lambda `[&count] { return ++count; }` hides three facts: what it
// stores (a pointer), who owns count, and where the closure lives. This file
// writes those facts out:
//
//   - The state is a named struct. Its members are the "captures".
//   - The behaviour is a free function that takes the state by reference.
//   - All counters live in one table of fixed size. The table owns them. The
//     caller decides where the table lives: on an embedded device, in static
//     storage, so that it uses no stack and no heap.
//
// Nothing here can dangle: no member is a pointer or a reference.
#pragma once

#include <array>
#include <cstddef>
#include <optional>

namespace e004 {

// The state of one counter. What a lambda would capture, with names.
struct Counter {
    int count = 0;
    int step = 1;
};

// Advances counter by its step. Returns the new count.
int counter_tick(Counter &counter);

inline constexpr std::size_t max_counters = 4;

// All counters of a program, in one block of fixed size.
struct Counter_table {
    std::array<Counter, max_counters> counters{};
    std::size_t used = 0;
};

// The size is part of the contract: a change that makes the table larger
// must show up here, not as a stack overflow on the device.
static_assert(sizeof(Counter_table) <= 48, "Counter_table grew; check the stack budget");

// Adds counter to table. Returns its index, or nothing when the table is full.
std::optional<std::size_t> counter_table_add(Counter_table &table, Counter counter);

// Ticks every counter in table, in the order they were added.
void counter_table_tick_all(Counter_table &table);

}  // namespace e004
