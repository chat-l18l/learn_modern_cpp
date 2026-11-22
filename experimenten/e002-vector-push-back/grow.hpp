// e002 — grow: append to a std::vector while you look at its elements.
//
// Contract of std::vector::push_back: when size() == capacity(), the vector
// moves to a new block. Every reference, pointer and iterator to an element
// is then invalid. When capacity is large enough, references and pointers to
// existing elements stay valid; only end() becomes invalid.
#pragma once

#include <cstddef>
#include <vector>

namespace e002 {

// Appends a copy of each even element of v to the end of v, in order.
// Returns the number of elements appended.
//
// Safe form of `for (int x : v) if (x % 2 == 0) v.push_back(x);`: the loop
// uses an index and a fixed count, so a reallocation cannot invalidate it.
std::size_t append_evens(std::vector<int> &v);

}  // namespace e002
