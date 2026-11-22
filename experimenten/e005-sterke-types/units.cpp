#include "units.hpp"

namespace e005 {

std::expected<Limits, Limits_error> limits_check(Limits limits)
{
    if (limits.low > limits.high) {
        return std::unexpected{Limits_error::low_above_high};
    }
    return limits;
}

bool limits_contain(Limits limits, Millivolt reading)
{
    return limits.low <= reading && reading <= limits.high;
}

}  // namespace e005
