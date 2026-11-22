#include "wrapper.hpp"

#include <minilib.hpp>
#include <numeric>
#include <utility>

namespace e003 {

std::optional<int> get(std::span<const int> values, std::size_t index)
{
    // ml_get takes an int index. std::in_range (C++20) checks that the value
    // fits in int, without the signed/unsigned traps of a hand-written test.
    if (index >= values.size() || !std::in_range<int>(index)) {
        return std::nullopt;
    }
    return ml_get(values.data(), static_cast<int>(index));
}

long long sum(std::span<const int> values)
{
    // long long: the sum of many ints can overflow int.
    return std::accumulate(values.begin(), values.end(), 0LL);
}

std::optional<unsigned> count(long v)
{
    if (!std::in_range<unsigned>(v)) {
        return std::nullopt;
    }
    return ml_count(v);
}

}  // namespace e003
