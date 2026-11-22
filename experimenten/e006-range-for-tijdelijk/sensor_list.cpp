#include "sensor_list.hpp"

#include <numeric>

namespace e006 {

Sensor_list make_sensor_list()
{
    return Sensor_list{.readings = {1, 2, 3, 4}};
}

std::optional<Sensor_list> make_optional_list(bool present)
{
    if (!present) {
        return std::nullopt;
    }
    return make_sensor_list();
}

const Readings &readings_of(const Sensor_list &list E006_LIFETIMEBOUND)
{
    return list.readings;
}

int sum_readings(const Readings &readings)
{
    return std::accumulate(readings.begin(), readings.end(), 0);
}

}  // namespace e006
