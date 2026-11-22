// Compile-fail test: must NOT compile. See CMakeLists.txt.
// A temperature passed where a voltage belongs.
#include "units.hpp"

bool check(e005::Limits limits)
{
    return e005::limits_contain(limits, e005::Deci_celsius{215});
}
