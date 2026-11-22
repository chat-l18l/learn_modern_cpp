// e005 — demo: two bugs WITHOUT undefined behaviour.
//
// THIS PROGRAM HAS BUGS ON PURPOSE. Do not fix them: they are the object of
// study. The safe forms are in sensor_pool.hpp and units.hpp.
//
// Every memory access here is valid, so ASan and UBSan have nothing to report.
// The output is deterministic, and the tests in CMakeLists.txt check it.
//
// Usage: e005_demo_bugs <stale-index|swapped-raw|swapped-strong>
//   stale-index     G28: a plain index after its slot was reused
//   swapped-raw     G29: alarm(int low_mv, int high_mv, ...) called swapped
//   swapped-strong  G29: the same with Millivolt; still compiles
#include "units.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

namespace {

// A pool with removal, but with a plain index instead of a handle.
struct Plain_pool {
    std::array<std::uint8_t, 4> channel{};
    std::array<bool, 4> used{};
};

std::uint16_t plain_add(Plain_pool &pool, std::uint8_t channel)
{
    for (std::uint16_t i = 0; i < pool.used.size(); ++i) {
        if (!pool.used.at(i)) {
            pool.used.at(i) = true;
            pool.channel.at(i) = channel;
            return i;
        }
    }
    return 0xFFFF;
}

void plain_remove(Plain_pool &pool, std::uint16_t index)
{
    pool.used.at(index) = false;
}

int demo_stale_index()
{
    Plain_pool pool;
    const std::uint16_t old_index = plain_add(pool, 1);
    plain_remove(pool, old_index);
    (void)plain_add(pool, 2);  // reuses the slot of old_index
    // old_index still points at a valid slot: no sanitizer can object.
    std::printf("read via old index: channel %d\n", pool.channel.at(old_index));
    return 0;
}

bool alarm_raw(int low_mv, int high_mv, int reading_mv)
{
    return reading_mv < low_mv || reading_mv > high_mv;
}

bool alarm_strong(e005::Millivolt low, e005::Millivolt high, e005::Millivolt reading)
{
    return reading < low || reading > high;
}

int demo_swapped_raw()
{
    const int low_mv = 100;
    const int high_mv = 900;
    // Swapped: 500 mV lies inside the band, but this raises an alarm.
    const bool alarm = alarm_raw(high_mv, low_mv, 500);
    std::printf("alarm for 500 mV: %s\n", alarm ? "yes" : "no");
    return 0;
}

int demo_swapped_strong()
{
    const e005::Millivolt low{100};
    const e005::Millivolt high{900};
    // Same swap with strong types. Both are Millivolt, so it compiles.
    const bool alarm = alarm_strong(high, low, e005::Millivolt{500});
    std::printf("alarm for 500 mV: %s\n", alarm ? "yes" : "no");
    return 0;
}

}  // namespace

int main(int argc, char **argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "stale-index";
    if (mode == "stale-index") {
        return demo_stale_index();
    }
    if (mode == "swapped-raw") {
        return demo_swapped_raw();
    }
    if (mode == "swapped-strong") {
        return demo_swapped_strong();
    }
    std::fprintf(stderr, "unknown mode '%.*s'\n", static_cast<int>(mode.size()), mode.data());
    return 2;
}
