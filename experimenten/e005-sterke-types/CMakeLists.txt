# e005 — strong types: handles, quantities and swapped parameters
add_library(e005_types STATIC units.cpp sensor_table.cpp sensor_pool.cpp)
leren_target(e005_types)
target_include_directories(e005_types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests. Part 1 of this file is static_assert: it runs while compiling.
leren_gtest(e005_test strong_types_test.cpp)
target_link_libraries(e005_test PRIVATE e005_types)

# Compile-fail tests: each source must NOT compile, and the compiler must
# give the expected reason. Table: source -> regex for GCC | Clang.
set(e005_fail_handle_of_other_table  "Actuator_id.*Sensor_id")
set(e005_fail_celsius_as_millivolt   "Deci_celsius.*Millivolt")
set(e005_fail_missing_field          "missing (initializer for member 'e005::Limits::high'|field 'high' initializer)")
set(e005_fail_designators_out_of_order
    "missing initializer for member 'e005::Limits::low'|field designators to be specified in declaration order")
foreach(case IN ITEMS handle_of_other_table celsius_as_millivolt missing_field designators_out_of_order)
    leren_compile_fail_test(e005_compile_fail_${case} compile_fail/${case}.cpp
                            "${e005_fail_${case}}")
endforeach()

# Demos of bugs without undefined behaviour. UNSAFE: the bugs are on purpose.
add_executable(e005_demo_bugs demo_bugs.cpp)
leren_target(e005_demo_bugs UNSAFE)
target_link_libraries(e005_demo_bugs PRIVATE e005_types)

# Blind-spot tests: the bug shows in the output, and no sanitizer reports it.
# Defined behaviour, so the output is the same under every preset.
set(e005_bug_expect_stale-index    "read via old index: channel 2")
set(e005_bug_expect_swapped-raw    "alarm for 500 mV: yes")
set(e005_bug_expect_swapped-strong "alarm for 500 mV: yes")
foreach(mode IN ITEMS stale-index swapped-raw swapped-strong)
    add_test(NAME e005_blind_${mode} COMMAND e005_demo_bugs ${mode})
    set_tests_properties(e005_blind_${mode} PROPERTIES
        PASS_REGULAR_EXPRESSION "${e005_bug_expect_${mode}}"
        FAIL_REGULAR_EXPRESSION "AddressSanitizer|runtime error|ThreadSanitizer"
        LABELS "blinde-vlek")
endforeach()
