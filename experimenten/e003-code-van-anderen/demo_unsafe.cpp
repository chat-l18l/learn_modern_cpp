// e003 — demo: call minilib (code of others) directly, without the wrapper.
//
// THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE. Do not fix it: it is the
// object of study. The safe form is in wrapper.cpp and wrapper_test.cpp.
//
// Usage: e003_demo_unsafe <sum|count|get>
//   sum    ml_sum over 3 values       V1 + V2: undefined behaviour
//   count  ml_count(-1)               V3: defined, but wrong (no UB)
//   get    ml_get at index 3 of 3     V4: undefined behaviour
//
// The same source also builds as e003_demo_unsafe_prebuilt, which links the
// uninstrumented minilib_prebuilt.
#include <cstdio>
#include <minilib.hpp>
#include <string_view>
#include <vector>

int main(int argc, char **argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "sum";
    const std::vector<int> v = {1, 2, 3};

    if (mode == "sum") {
        std::printf("ml_sum=%d\n", ml_sum(v.data(), 3));  // reads v[3]
        return 0;
    }
    if (mode == "count") {
        std::printf("ml_count(-1)=%u\n", ml_count(-1));  // wraps to UINT_MAX
        return 0;
    }
    if (mode == "get") {
        std::printf("ml_get(3)=%d\n", ml_get(v.data(), 3));  // reads v[3]
        return 0;
    }
    std::fprintf(stderr, "unknown mode '%.*s'\n", static_cast<int>(mode.size()), mode.data());
    return 2;
}
