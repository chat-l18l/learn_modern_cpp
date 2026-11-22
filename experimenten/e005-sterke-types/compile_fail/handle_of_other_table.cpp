// Compile-fail test: must NOT compile. See CMakeLists.txt.
// An Actuator_id passed where a Sensor_id belongs.
#include "sensor_table.hpp"

int read(const e005::Sensor_table &table)
{
    return e005::sensor_at(table, e005::Actuator_id{0}).channel;
}
