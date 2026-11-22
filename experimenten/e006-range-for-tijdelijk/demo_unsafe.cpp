// e006 — demo: range-for over a part of a temporary.
//
// THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE, depending on the C++
// standard it is built with. Do not fix it: it is the object of study.
// CMakeLists.txt builds it twice: e006_demo_unsafe_cpp20 and _cpp23.
//
// Usage: e006_demo_unsafe_cppNN <mode>
//   direct-member   for (x : make().readings)            correct in C++20 and C++23
//   via-function    for (x : readings_of(make()))        UB in C++20, correct in C++23
//   optional-value  for (x : make_optional().value()...) UB in C++20, correct in C++23
//   named-ref       const auto &r = readings_of(make()); UB in C++20 and C++23
//
// Prints the sum of the readings; 10 is the correct answer.
#include "sensor_list.hpp"

#include <cstdio>
#include <string_view>

namespace {

int sum_direct_member()
{
    int sum = 0;
    for (const int x : e006::make_sensor_list().readings) {
        sum += x;
    }
    return sum;
}

int sum_via_function()
{
    int sum = 0;
    for (const int x : e006::readings_of(e006::make_sensor_list())) {
        sum += x;
    }
    return sum;
}

int sum_optional_value()
{
    int sum = 0;
    for (const int x : e006::make_optional_list(true).value().readings) {
        sum += x;
    }
    return sum;
}

int sum_named_ref()
{
    // The "tidy" rewrite of sum_via_function. P2718 does not cover it.
    const auto &readings = e006::readings_of(e006::make_sensor_list());
    int sum = 0;
    for (const int x : readings) {
        sum += x;
    }
    return sum;
}

}  // namespace

int main(int argc, char **argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "via-function";
    int sum = -1;
    if (mode == "direct-member") {
        sum = sum_direct_member();
    } else if (mode == "via-function") {
        sum = sum_via_function();
    } else if (mode == "optional-value") {
        sum = sum_optional_value();
    } else if (mode == "named-ref") {
        sum = sum_named_ref();
    } else {
        std::fprintf(stderr, "unknown mode '%.*s'\n", static_cast<int>(mode.size()), mode.data());
        return 2;
    }
    std::printf("C++%ld %.*s: sum %d\n", __cplusplus / 100 % 100, static_cast<int>(mode.size()),
                mode.data(), sum);
    return sum == 10 ? 0 : 1;
}
