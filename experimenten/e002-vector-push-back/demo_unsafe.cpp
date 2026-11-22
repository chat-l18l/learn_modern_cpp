// e002 — demo: use a reference, iterator or element after push_back.
//
// THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE. Do not fix it: it is the
// object of study. The safe forms are in grow.cpp and grow_test.cpp.
//
// Usage: e002_demo_unsafe <reference|iterator|range-for|spare>
//   reference  int& to v[0], then push_back             (undefined behaviour)
//   iterator   v.begin(), then push_back                (undefined behaviour)
//   range-for  push_back inside `for (int x : v)`       (undefined behaviour)
//   spare      reserve(4) first, then as reference      (correct: control)
//
// The program prints size and capacity, so that the output shows whether
// push_back reallocated.
#include <cstdio>
#include <string_view>
#include <vector>

namespace {

void show(const char *label, const std::vector<int> &v)
{
    std::printf("%-8s size=%zu capacity=%zu\n", label, v.size(), v.capacity());
}

int demo_reference(bool reserve_first)
{
    std::vector<int> v = {1, 2, 3};
    if (reserve_first) {
        v.reserve(4);
    }
    show("before", v);
    int &first = v[0];
    v.push_back(4);
    show("after", v);
    // Without reserve: `first` refers to the freed block.
    std::printf("first=%d\n", first);
    return first == 1 ? 0 : 1;
}

int demo_iterator()
{
    std::vector<int> v = {1, 2, 3};
    show("before", v);
    auto it = v.begin();
    v.push_back(4);
    show("after", v);
    const int value = *it;  // `it` points into the freed block
    std::printf("*it=%d\n", value);
    return value == 1 ? 0 : 1;
}

int demo_range_for()
{
    std::vector<int> v = {1, 2, 3};
    show("before", v);
    // The range-for keeps begin() and end() of the old block. The first
    // push_back frees that block; the loop then reads freed memory.
    for (const int x : v) {
        if (x % 2 == 0) {
            v.push_back(x);
        }
    }
    show("after", v);
    std::printf("v =");
    for (const int x : v) {
        std::printf(" %d", x);
    }
    std::printf("\n");
    return v == std::vector<int>{1, 2, 3, 2} ? 0 : 1;
}

}  // namespace

int main(int argc, char **argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "reference";
    if (mode == "reference") {
        return demo_reference(false);
    }
    if (mode == "spare") {
        return demo_reference(true);
    }
    if (mode == "iterator") {
        return demo_iterator();
    }
    if (mode == "range-for") {
        return demo_range_for();
    }
    std::fprintf(stderr, "unknown mode '%.*s'\n", static_cast<int>(mode.size()), mode.data());
    return 2;
}
