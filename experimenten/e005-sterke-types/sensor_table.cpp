#include "sensor_table.hpp"

#include <cassert>

namespace e005 {

std::expected<Sensor_id, Table_error> sensor_add(Sensor_table &table, Sensor sensor)
{
    assert(table.used <= table.sensors.size());
    if (table.used == table.sensors.size()) {
        return std::unexpected{Table_error::full};
    }
    const Sensor_id id{table.used};
    table.sensors.at(table.used) = sensor;
    ++table.used;
    return id;
}

const Sensor &sensor_at(const Sensor_table &table, Sensor_id id)
{
    assert(id.value < table.used);  // ids come from sensor_add or sensor_find
    return table.sensors.at(id.value);
}

std::expected<Sensor_id, Table_error> sensor_find(const Sensor_table &table, std::uint16_t raw)
{
    if (raw >= table.used) {
        return std::unexpected{Table_error::unknown_id};
    }
    return Sensor_id{raw};
}

}  // namespace e005
