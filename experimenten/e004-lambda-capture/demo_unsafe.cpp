// e004 — demo: a lambda that outlives what it captured.
//
// THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE. Do not fix it: it is the
// object of study. The transparent alternative is in counter.hpp.
//
// Usage: e004_demo_unsafe <escape-ref|escape-this>
//   escape-ref   [&count] escapes from the function that owns count
//   escape-this  [=] in a member function captures `this`; the object dies
#include <cstdio>
#include <functional>
#include <string_view>

namespace {

// count lives in the stack frame of make_counter. The closure stores only
// the address of count. The frame is gone when make_counter returns.
std::function<int()> make_counter()
{
    int count = 0;
    return [&count] { return ++count; };
}

struct Sensor {
    int value = 42;

    // [=] looks like "copy everything", but `value` is a member: the lambda
    // copies `this`, not `value`.
    std::function<int()> reader()
    {
        return [=] { return value; };
    }
};

int demo_escape_ref()
{
    const std::function<int()> tick = make_counter();
    const int first = tick();  // writes to the dead frame of make_counter
    const int second = tick();
    std::printf("tick: %d %d\n", first, second);
    return (first == 1 && second == 2) ? 0 : 1;
}

int demo_escape_this()
{
    std::function<int()> read;
    {
        Sensor sensor;
        read = sensor.reader();
    }  // sensor ends here; read still holds its address
    const int value = read();
    std::printf("value: %d\n", value);
    return value == 42 ? 0 : 1;
}

}  // namespace

int main(int argc, char **argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "escape-ref";
    if (mode == "escape-ref") {
        return demo_escape_ref();
    }
    if (mode == "escape-this") {
        return demo_escape_this();
    }
    std::fprintf(stderr, "unknown mode '%.*s'\n", static_cast<int>(mode.size()), mode.data());
    return 2;
}
