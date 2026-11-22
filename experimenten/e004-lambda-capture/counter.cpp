#include "counter.hpp"

#include <cassert>

namespace e004 {

int counter_tick(Counter &counter)
{
    counter.count += counter.step;
    return counter.count;
}

std::optional<std::size_t> counter_table_add(Counter_table &table, Counter counter)
{
    assert(table.used <= table.counters.size());  // invariant of Counter_table
    if (table.used == table.counters.size()) {
        return std::nullopt;  // full: an operational condition, not a bug
    }
    const std::size_t index = table.used;
    table.counters.at(index) = counter;
    ++table.used;
    return index;
}

void counter_table_tick_all(Counter_table &table)
{
    assert(table.used <= table.counters.size());
    for (std::size_t i = 0; i < table.used; ++i) {
        counter_tick(table.counters.at(i));
    }
}

}  // namespace e004
