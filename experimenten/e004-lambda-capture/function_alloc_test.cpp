// e004 — measurement: when does std::function use the heap?
//
// This test binary replaces the global operator new to count allocations.
// It is a separate binary, so that the counter does not touch other tests.
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <new>

namespace {
std::size_t s_allocations = 0;
}  // namespace

// NOLINTBEGIN(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
void *operator new(std::size_t size)
{
    ++s_allocations;
    if (void *p = std::malloc(size == 0 ? 1 : size)) {
        return p;
    }
    throw std::bad_alloc{};
}

void operator delete(void *p) noexcept
{
    std::free(p);
}

void operator delete(void *p, std::size_t /*size*/) noexcept
{
    std::free(p);
}
// NOLINTEND(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)

namespace {

// Returns the number of heap allocations that building a std::function from
// a closure with `capture_bytes` bytes of captures costs.
template <std::size_t capture_bytes> std::size_t allocations_for_capture()
{
    const std::array<std::byte, capture_bytes> data{};
    const std::size_t before = s_allocations;
    const std::function<std::size_t()> f = [data] { return data.size(); };
    const std::size_t after = s_allocations;
    EXPECT_EQ(f(), capture_bytes);
    // cppcheck-suppress duplicateExpression ; operator new changes s_allocations
    return after - before;
}

TEST(StdFunctionHeap, SmallCaptureStaysInside)
{
    EXPECT_EQ(allocations_for_capture<8>(), 0U);
    EXPECT_EQ(allocations_for_capture<16>(), 0U);
}

TEST(StdFunctionHeap, LargerCaptureGoesToTheHeap)
{
    EXPECT_EQ(allocations_for_capture<17>(), 1U);
    EXPECT_EQ(allocations_for_capture<4096>(), 1U);
}

TEST(StdFunctionSize, IsFourPointersOnLibstdcxx)
{
    // The std::function object itself: 16 bytes of inline buffer plus two
    // pointers. Implementation-specific; measured on libstdc++ 15, x86-64.
    EXPECT_EQ(sizeof(std::function<int()>), 32U);
}

TEST(StdFunctionHeap, ReferenceCaptureStaysInside)
{
    std::array<std::byte, 4096> big{};
    const std::size_t before = s_allocations;
    const std::function<std::size_t()> f = [&big] { return big.size(); };
    // cppcheck-suppress duplicateExpression ; operator new changes s_allocations
    EXPECT_EQ(s_allocations - before, 0U);
    EXPECT_EQ(f(), big.size());
}

}  // namespace
