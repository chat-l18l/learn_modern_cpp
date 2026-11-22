# e003 — code of others: warnings hidden by -isystem, blind spots of ASan
add_library(e003_wrapper STATIC wrapper.cpp)
leren_target(e003_wrapper)
target_include_directories(e003_wrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(e003_wrapper PRIVATE minilib)

# Unit tests of the wrapper. Must pass under every preset.
leren_gtest(e003_test wrapper_test.cpp)
target_link_libraries(e003_test PRIVATE e003_wrapper)

# The demos call minilib directly and have undefined behaviour on purpose.
add_executable(e003_demo_unsafe demo_unsafe.cpp)
leren_target(e003_demo_unsafe UNSAFE)
target_link_libraries(e003_demo_unsafe PRIVATE minilib)

add_executable(e003_demo_unsafe_prebuilt demo_unsafe.cpp)
leren_target(e003_demo_unsafe_prebuilt UNSAFE)
target_link_libraries(e003_demo_unsafe_prebuilt PRIVATE minilib_prebuilt)

if(LEREN_SANITIZE STREQUAL "address")
    # Safety-net tests: ASan must catch the read past the end, both in inline
    # header code (sum) and in the library built from source (get).
    foreach(mode IN ITEMS sum get)
        add_test(NAME e003_asan_catches_${mode} COMMAND e003_demo_unsafe ${mode})
        set_tests_properties(e003_asan_catches_${mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "AddressSanitizer: heap-buffer-overflow"
            LABELS "vangnet")
    endforeach()

    # Blind-spot test: the same read inside an uninstrumented library is NOT
    # seen by ASan. The test passes while ASan stays silent. If a future ASan
    # sees it, the test fails, and the report must be updated.
    add_test(NAME e003_asan_blind_to_prebuilt COMMAND e003_demo_unsafe_prebuilt get)
    set_tests_properties(e003_asan_blind_to_prebuilt PROPERTIES
        FAIL_REGULAR_EXPRESSION "AddressSanitizer"
        LABELS "blinde-vlek")
endif()
