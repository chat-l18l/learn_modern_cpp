// e004 — tests for the transparent alternative, and measurements of where
// lambda captures live.
#include "counter.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

namespace {

// --- Where the captures live -------------------------------------------

TEST(ClosureSize, ReferenceCaptureIsOnePointer)
{
    int count = 0;
    auto tick = [&count] { return ++count; };
    EXPECT_EQ(sizeof(tick), sizeof(int *));
    EXPECT_EQ(tick(), 1);
}

TEST(ClosureSize, ValueCaptureIsACopy)
{
    const std::array<int, 16> values{};
    auto read = [values] { return values[0]; };
    EXPECT_EQ(sizeof(read), sizeof(values));
    EXPECT_EQ(read(), 0);
}

TEST(ClosureSize, LambdaIsTheSameSizeAsTheStructWrittenOut)
{
    // What the compiler generates for [&count, step]. Written out here only
    // to compare sizes.
    struct Tick_written_out {
        int *count;
        int step;
    };
    int count = 0;
    // Not const: a const int with a constant value needs no capture at all,
    // and Clang then reports -Wunused-lambda-capture.
    int step = 2;
    auto tick = [&count, step] { return count += step; };
    EXPECT_EQ(sizeof(tick), sizeof(Tick_written_out));
    EXPECT_EQ(tick(), 2);
}

TEST(ClosureSize, EmptyCaptureStillTakesOneByte)
{
    auto nothing = [] { return 0; };
    EXPECT_EQ(sizeof(nothing), 1U);  // every object has a size of at least 1
    EXPECT_EQ(nothing(), 0);
}

// --- The transparent alternative ---------------------------------------

TEST(Counter, TickAddsTheStep)
{
    e004::Counter counter{.count = 10, .step = 5};
    EXPECT_EQ(e004::counter_tick(counter), 15);
    EXPECT_EQ(counter.count, 15);
}

TEST(CounterTable, TicksEveryCounterInOrder)
{
    e004::Counter_table table;
    ASSERT_EQ(e004::counter_table_add(table, {.count = 0, .step = 1}), 0U);
    ASSERT_EQ(e004::counter_table_add(table, {.count = 100, .step = 10}), 1U);
    e004::counter_table_tick_all(table);
    e004::counter_table_tick_all(table);
    EXPECT_EQ(table.counters.at(0).count, 2);
    EXPECT_EQ(table.counters.at(1).count, 120);
}

TEST(CounterTable, RefusesACounterWhenFull)
{
    e004::Counter_table table;
    for (std::size_t i = 0; i < e004::max_counters; ++i) {
        ASSERT_TRUE(e004::counter_table_add(table, {}).has_value());
    }
    EXPECT_FALSE(e004::counter_table_add(table, {}).has_value());
    EXPECT_EQ(table.used, e004::max_counters);
}

TEST(CounterTable, OutlivesTheFunctionThatFilledIt)
{
    // The escape-ref demo fails here: its state died with the function. The
    // table owns its state, so filling it in another function is safe.
    auto fill = [](e004::Counter_table &table) {
        const e004::Counter local{.count = 0, .step = 3};
        (void)e004::counter_table_add(table, local);  // copied into the table
    };
    e004::Counter_table table;
    fill(table);
    e004::counter_table_tick_all(table);
    EXPECT_EQ(table.counters.at(0).count, 3);
}

}  // namespace
