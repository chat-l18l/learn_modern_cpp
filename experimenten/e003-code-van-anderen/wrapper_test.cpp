// e003 — tests for the wrapper around minilib.
#include "wrapper.hpp"

#include <gtest/gtest.h>

#include <climits>
#include <cstddef>
#include <vector>

namespace {

TEST(WrapperGet, ReturnsTheValueInRange)
{
    const std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(e003::get(v, 2), 3);
}

TEST(WrapperGet, RefusesTheIndexOnePastTheEnd)
{
    const std::vector<int> v = {1, 2, 3};
    EXPECT_FALSE(e003::get(v, 3).has_value());
}

TEST(WrapperGet, RefusesAnIndexThatDoesNotFitInInt)
{
    const std::vector<int> v = {1, 2, 3};
    EXPECT_FALSE(e003::get(v, static_cast<std::size_t>(INT_MAX) + 1).has_value());
}

TEST(WrapperGet, RefusesAnyIndexOnAnEmptySpan)
{
    EXPECT_FALSE(e003::get({}, 0).has_value());
}

TEST(WrapperSum, AddsAllValues)
{
    const std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(e003::sum(v), 6);
}

TEST(WrapperSum, DoesNotOverflowForLargeValues)
{
    const std::vector<int> v = {INT_MAX, INT_MAX};
    EXPECT_EQ(e003::sum(v), 2LL * INT_MAX);
}

TEST(WrapperSum, ReturnsZeroForAnEmptySpan)
{
    EXPECT_EQ(e003::sum({}), 0);
}

TEST(WrapperCount, PassesAValueThatFits)
{
    EXPECT_EQ(e003::count(42), 42U);
}

TEST(WrapperCount, RefusesANegativeValue)
{
    EXPECT_FALSE(e003::count(-1).has_value());
}

TEST(WrapperCount, RefusesAValueAboveUintMax)
{
    EXPECT_FALSE(e003::count(static_cast<long>(UINT_MAX) + 1).has_value());
}

}  // namespace
