// Compile-fail test: must NOT compile. See CMakeLists.txt.
// A Limits without its high field.
#include "units.hpp"

e005::Limits make()
{
    return e005::Limits{.low = e005::Millivolt{100}};
}
