// Compile-fail test: must NOT compile. See CMakeLists.txt.
// Designated initializers in another order than the declaration.
#include "units.hpp"

e005::Limits make()
{
    return e005::Limits{.high = e005::Millivolt{900}, .low = e005::Millivolt{100}};
}
