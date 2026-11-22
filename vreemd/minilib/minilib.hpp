// minilib — pretend third-party library, C style.
//
// THIS CODE HAS BUGS ON PURPOSE (V1-V4, see vreemd/README.md). Do not fix
// them: they are the object of study in e003.
#pragma once

// Returns the sum of the first n values of p.
inline int ml_sum(const int *p, int n)
{
    int s;                          // V1: not initialised
    for (int i = 0; i <= n; ++i) {  // V2: reads p[n], one past the end
        s += p[i];
    }
    return s;
}

// Returns a count as unsigned.
inline unsigned ml_count(long v)
{
    return v;  // V3: silent narrowing, and a negative v becomes huge
}

// Returns p[i]. The caller must make sure that i is in range.
int ml_get(const int *p, int i);  // V4: no bounds check, see minilib.cpp
