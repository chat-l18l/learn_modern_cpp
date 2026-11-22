// e002 — tests for the safe forms.
//
// Each test makes the vector full (size() == capacity()) before push_back, so
// that a reallocation really happens. Without that, a test can pass for the
// wrong reason.
#include "grow.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

// Returns a vector with exactly these elements and no spare capacity.
std::vector<int> full_vector(std::vector<int> v)
{
    v.shrink_to_fit();  // a request, not a promise; the tests check it
    return v;
}

TEST(Setup, FullVectorHasNoSpareCapacity)
{
    const auto v = full_vector({1, 2, 3});
    EXPECT_EQ(v.capacity(), v.size());
}

TEST(Reallocation, HappensWhenTheVectorIsFull)
{
    // The standard: push_back reallocates when the new size exceeds the
    // capacity, and only then. A larger capacity proves a new block.
    // Comparing v.data() with an address from before push_back would use an
    // invalid pointer value; Clang and cppcheck both report that.
    auto v = full_vector({1, 2, 3});
    const std::size_t before = v.capacity();
    ASSERT_EQ(before, v.size());
    v.push_back(4);
    EXPECT_GT(v.capacity(), before);
}

TEST(SafeIndex, StaysValidAfterReallocation)
{
    auto v = full_vector({1, 2, 3});
    ASSERT_EQ(v.capacity(), v.size());
    const std::size_t first = 0;
    v.push_back(4);
    EXPECT_EQ(v[first], 1);
}

// This test is correct code, and both Clang 23 (-Wlifetime-safety-invalidation)
// and cppcheck 2.22 (invalidContainer) report it. Neither tool knows the
// capacity, so a reserve() that makes the code correct looks the same to them
// as the bug. Measured in e002; see README.md.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlifetime-safety-invalidation"
#endif
TEST(SafeReserve, KeepsTheAddressWhenCapacityIsEnough)
{
    std::vector<int> v = {1, 2, 3};
    v.reserve(4);
    const int &first = v[0];
    const int *before = &first;
    v.push_back(4);
    // cppcheck-suppress invalidContainer ; false positive, see above
    EXPECT_EQ(v.data(), before);
    EXPECT_EQ(first, 1);
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

TEST(SafeReferenceAfterChange, TakesTheReferenceAfterPushBack)
{
    auto v = full_vector({1, 2, 3});
    v.push_back(4);
    const int &first = v[0];
    EXPECT_EQ(first, 1);
}

TEST(AppendEvens, AppendsACopyOfEachEvenElement)
{
    auto v = full_vector({1, 2, 3, 4});
    ASSERT_EQ(v.capacity(), v.size());
    EXPECT_EQ(e002::append_evens(v), 2U);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 2, 4}));
}

TEST(AppendEvens, DoesNothingWithoutEvenElements)
{
    auto v = full_vector({1, 3, 5});
    EXPECT_EQ(e002::append_evens(v), 0U);
    EXPECT_EQ(v, (std::vector<int>{1, 3, 5}));
}

TEST(AppendEvens, AcceptsAnEmptyVector)
{
    std::vector<int> v;
    EXPECT_EQ(e002::append_evens(v), 0U);
    EXPECT_TRUE(v.empty());
}

TEST(AppendEvens, VisitsOnlyTheOriginalElements)
{
    // Every element is even. A loop that also visits the appended elements
    // never stops.
    auto v = full_vector({2, 4});
    EXPECT_EQ(e002::append_evens(v), 2U);
    EXPECT_EQ(v, (std::vector<int>{2, 4, 2, 4}));
}

}  // namespace
