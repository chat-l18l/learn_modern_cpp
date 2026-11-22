# e002 — reference into a std::vector after push_back
add_library(e002_grow STATIC grow.cpp)
leren_target(e002_grow)
target_include_directories(e002_grow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests of the safe forms. Must pass under every preset.
leren_gtest(e002_test grow_test.cpp)
target_link_libraries(e002_test PRIVATE e002_grow)

# The demo has undefined behaviour on purpose. It uses only std::vector, so
# it links nothing else. That matters for the debug-mode build below: debug
# mode changes the layout of std::vector, and must not meet code built
# without it.
add_executable(e002_demo_unsafe demo_unsafe.cpp)
leren_target(e002_demo_unsafe UNSAFE)

add_executable(e002_demo_unsafe_debugmode demo_unsafe.cpp)
leren_target(e002_demo_unsafe_debugmode UNSAFE)
target_compile_definitions(e002_demo_unsafe_debugmode PRIVATE _GLIBCXX_DEBUG)

# Control: `spare` is correct code. It must succeed under every preset, also
# under ASan. A report here would be a false alarm.
add_test(NAME e002_spare_is_correct COMMAND e002_demo_unsafe spare)
set_tests_properties(e002_spare_is_correct PROPERTIES LABELS "controle")

# Safety-net tests for the libstdc++ debug mode. Debug mode checks iterators,
# not references, so `reference` has no test here: it is not caught.
#
# Debug mode stops the program with abort(). CTest counts a signal as a
# failure even when PASS_REGULAR_EXPRESSION matches, so the test runs the demo
# through `sh -c "... || true"`. Only the report text then decides.
set(e002_debugmode_expect_iterator  "attempt to dereference a singular iterator")
set(e002_debugmode_expect_range-for "attempt to increment a singular iterator")

if(LEREN_SANITIZE STREQUAL "none")
    foreach(mode IN ITEMS iterator range-for)
        add_test(NAME e002_debugmode_catches_${mode}
                 COMMAND sh -c "\"$<TARGET_FILE:e002_demo_unsafe_debugmode>\" ${mode} || true")
        set_tests_properties(e002_debugmode_catches_${mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "${e002_debugmode_expect_${mode}}"
            LABELS "vangnet")
    endforeach()
endif()

# Safety-net tests under ASan. Table: demo mode -> report ASan must print.
set(e002_asan_expect_reference "AddressSanitizer: heap-use-after-free")
set(e002_asan_expect_iterator  "AddressSanitizer: heap-use-after-free")
set(e002_asan_expect_range-for "AddressSanitizer: heap-use-after-free")

if(LEREN_SANITIZE STREQUAL "address")
    foreach(mode IN ITEMS reference iterator range-for)
        add_test(NAME e002_asan_catches_${mode} COMMAND e002_demo_unsafe ${mode})
        set_tests_properties(e002_asan_catches_${mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "${e002_asan_expect_${mode}}"
            LABELS "vangnet")
    endforeach()
endif()
