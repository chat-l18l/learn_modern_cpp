# @ID@ — @TITEL@
add_library(@ID@_lib STATIC example.cpp)
leren_target(@ID@_lib)
target_include_directories(@ID@_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests of the safe form. Must pass under every preset.
leren_gtest(@ID@_test example_test.cpp)
target_link_libraries(@ID@_test PRIVATE @ID@_lib)

# The demo has undefined behaviour on purpose.
add_executable(@ID@_demo_unsafe demo_unsafe.cpp)
leren_target(@ID@_demo_unsafe UNSAFE)
target_link_libraries(@ID@_demo_unsafe PRIVATE @ID@_lib)

# Safety-net test: passes only when the sanitizer prints the expected report.
# Fill in the exact report text after you have seen it once. Until the demo
# really has undefined behaviour, this test fails: that is on purpose.
if(LEREN_SANITIZE STREQUAL "address")
    add_test(NAME @ID@_asan_catches COMMAND @ID@_demo_unsafe)
    set_tests_properties(@ID@_asan_catches PROPERTIES
        PASS_REGULAR_EXPRESSION "AddressSanitizer: "
        LABELS "vangnet")
endif()
