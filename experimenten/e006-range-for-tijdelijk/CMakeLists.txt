# e006 — range-for over a part of a temporary (C++20 versus C++23)
add_library(e006_sensor_list STATIC sensor_list.cpp)
leren_target(e006_sensor_list)
target_include_directories(e006_sensor_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests of the forms that are correct in both standards.
leren_gtest(e006_test sensor_list_test.cpp)
target_link_libraries(e006_test PRIVATE e006_sensor_list)

# The demo, built once per standard. UNSAFE: its warnings are measurements.
foreach(std IN ITEMS 20 23)
    add_executable(e006_demo_unsafe_cpp${std} demo_unsafe.cpp)
    set_target_properties(e006_demo_unsafe_cpp${std} PROPERTIES CXX_STANDARD ${std})
    leren_target(e006_demo_unsafe_cpp${std} UNSAFE)
    target_link_libraries(e006_demo_unsafe_cpp${std} PRIVATE e006_sensor_list)
endforeach()

# Controls: correct code, so these must succeed under every preset, and no
# sanitizer may report anything. Table: standard -> modes that are correct.
set(e006_correct_in_20 direct-member)
set(e006_correct_in_23 direct-member via-function optional-value)
foreach(std IN ITEMS 20 23)
    foreach(mode IN LISTS e006_correct_in_${std})
        add_test(NAME e006_correct_cpp${std}_${mode} COMMAND e006_demo_unsafe_cpp${std} ${mode})
        set_tests_properties(e006_correct_cpp${std}_${mode} PROPERTIES
            FAIL_REGULAR_EXPRESSION "AddressSanitizer|runtime error"
            LABELS "controle")
    endforeach()
endforeach()

# Safety-net tests: undefined behaviour that ASan must catch.
# Table: standard -> modes with undefined behaviour.
set(e006_ub_in_20 via-function optional-value named-ref)
set(e006_ub_in_23 named-ref)
if(LEREN_SANITIZE STREQUAL "address")
    foreach(std IN ITEMS 20 23)
        foreach(mode IN LISTS e006_ub_in_${std})
            add_test(NAME e006_asan_catches_cpp${std}_${mode}
                     COMMAND e006_demo_unsafe_cpp${std} ${mode})
            set_tests_properties(e006_asan_catches_cpp${std}_${mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "AddressSanitizer: stack-use-after-scope"
                LABELS "vangnet")
        endforeach()
    endforeach()
endif()
