#include "grow.hpp"

namespace e002 {

// Clang 23 reports -Wlifetime-safety-invalidation on this function: "parameter
// 'v' is invalidated by push_back, later used in v[i]". That is a false
// positive. push_back invalidates references to elements, not the reference
// to the vector itself, and v[i] makes a new reference on every iteration.
// Measured in e002; see README.md.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlifetime-safety-invalidation"
#endif
std::size_t append_evens(std::vector<int> &v)
{
    // Count first: the loop must not visit the elements that it appends.
    const std::size_t original_size = v.size();
    std::size_t appended = 0;
    for (std::size_t i = 0; i < original_size; ++i) {
        // Copy the value before push_back. `v.push_back(v[i])` is also
        // correct (the standard requires it), but the copy makes it obvious.
        const int value = v[i];
        if (value % 2 == 0) {
            v.push_back(value);
            ++appended;
        }
    }
    return appended;
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace e002
