// e004 — demo: how much stack a capture costs.
//
// No undefined behaviour here. This file exists to be measured with
// -fstack-usage and -Wframe-larger-than=1024 (see CMakeLists.txt). The
// warnings it produces are the result of the experiment.
#include <array>
#include <cstddef>
#include <cstdio>

namespace {

constexpr std::size_t buffer_size = 4096;

// [buffer] copies 4 KiB into the closure object. The closure lives in this
// stack frame, next to the original buffer: two copies on the stack.
int read_first_by_value()
{
    std::array<std::byte, buffer_size> buffer{};
    buffer[0] = std::byte{7};
    auto read = [buffer] { return static_cast<int>(buffer[0]); };
    return read();
}

// [&buffer] stores one pointer. Only the original buffer is on the stack.
int read_first_by_reference()
{
    std::array<std::byte, buffer_size> buffer{};
    buffer[0] = std::byte{7};
    auto read = [&buffer] { return static_cast<int>(buffer[0]); };
    return read();
}

// The buffer is not on the stack at all: it has static storage. This is the
// usual choice on a device with a small stack.
std::array<std::byte, buffer_size> s_buffer{};

int read_first_static()
{
    s_buffer[0] = std::byte{7};
    return static_cast<int>(s_buffer[0]);
}

}  // namespace

int main()
{
    std::printf("%d %d %d\n", read_first_by_value(), read_first_by_reference(),
                read_first_static());
    return 0;
}
