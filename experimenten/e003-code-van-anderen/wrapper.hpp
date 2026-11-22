// e003 — wrapper: a safe boundary around minilib (code of others).
//
// The wrapper is the only place in our code that calls minilib. It takes
// C++ types (std::span, std::size_t), checks every precondition of minilib,
// and reports a failed check as an empty std::optional. Code behind the
// wrapper never sees a raw pointer or an unchecked index.
#pragma once

#include <cstddef>
#include <optional>
#include <span>

namespace e003 {

// Returns values[index], or nothing when index is out of range.
// Calls ml_get only after the range check (minilib bug V4).
std::optional<int> get(std::span<const int> values, std::size_t index);

// Returns the sum of values. Does NOT call ml_sum: ml_sum is broken
// (minilib bugs V1 and V2). A wrapper may refuse a function of others.
long long sum(std::span<const int> values);

// Returns v as unsigned, or nothing when v does not fit.
// Calls ml_count only for values that fit (minilib bug V3).
std::optional<unsigned> count(long v);

}  // namespace e003
