// e006 — tests for the forms that are correct in C++20 AND C++23.
//
// The tests do not use the form that is only correct since C++23
// (`for (x : f(temporary))`); that form is measured in the demo instead.
#include "sensor_list.hpp"

#include <gtest/gtest.h>

namespace {

TEST(RangeFor, OverANamedOwner)
{
    const e006::Sensor_list list = e006::make_sensor_list();  // the owner has a name
    int sum = 0;
    for (const int x : e006::readings_of(list)) {
        // cppcheck-suppress useStlAlgorithm ; the range-for is the subject of the test
        sum += x;
    }
    EXPECT_EQ(sum, 10);
}

TEST(RangeFor, OverAMemberOfATemporary)
{
    // Correct since C++11: a reference to a member extends the whole temporary.
    int sum = 0;
    for (const int x : e006::make_sensor_list().readings) {
        // cppcheck-suppress useStlAlgorithm ; the range-for is the subject of the test
        sum += x;
    }
    EXPECT_EQ(sum, 10);
}

TEST(RangeFor, CopyTheValueInsideTheFullExpression)
{
    // The copy happens before the temporary dies at the ;. The loop then
    // runs over a value with a name, owned by this function.
    const e006::Readings readings = e006::readings_of(e006::make_sensor_list());
    EXPECT_EQ(e006::sum_readings(readings), 10);
}

TEST(RangeFor, OptionalWithANamedOwner)
{
    const auto list = e006::make_optional_list(true);
    ASSERT_TRUE(list.has_value());
    EXPECT_EQ(e006::sum_readings(list->readings), 10);
}

TEST(RangeFor, OptionalCanBeEmpty)
{
    EXPECT_FALSE(e006::make_optional_list(false).has_value());
}

}  // namespace
