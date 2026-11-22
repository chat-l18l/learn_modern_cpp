// minilib — pretend third-party library. Bugs on purpose; see minilib.hpp.
#include "minilib.hpp"

int ml_get(const int *p, int i)
{
    return p[i];  // V4: no bounds check
}
