# e004 — lambda with a reference capture that outlives its variable
add_library(e004_counter STATIC counter.cpp)
leren_target(e004_counter)
target_include_directories(e004_counter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests of the transparent alternative, and closure sizes.
leren_gtest(e004_test counter_test.cpp)
target_link_libraries(e004_test PRIVATE e004_counter)

# Measurement of std::function heap use. Own binary: it replaces operator new.
# Not under a sanitizer: ASan and TSan replace operator new themselves, and
# the two replacements mix (ASan reports alloc-dealloc-mismatch in gtest).
if(LEREN_SANITIZE STREQUAL "none")
    leren_gtest(e004_function_alloc_test function_alloc_test.cpp)
endif()

# The demo has undefined behaviour on purpose.
add_executable(e004_demo_unsafe demo_unsafe.cpp)
leren_target(e004_demo_unsafe UNSAFE)

# Stack measurement. No undefined behaviour, but it must produce warnings,
# so it is UNSAFE (no -Werror). -fstack-usage writes a .su file per object
# file with the frame size of each function.
add_executable(e004_demo_stack demo_stack.cpp)
leren_target(e004_demo_stack UNSAFE)
target_compile_options(e004_demo_stack PRIVATE -fstack-usage -Wframe-larger-than=1024)

if(LEREN_SANITIZE STREQUAL "address")
    # escape-this: the object ended in an inner scope.
    add_test(NAME e004_asan_catches_escape-this COMMAND e004_demo_unsafe escape-this)
    set_tests_properties(e004_asan_catches_escape-this PROPERTIES
        PASS_REGULAR_EXPRESSION "AddressSanitizer: stack-use-after-scope"
        LABELS "vangnet")

    # escape-ref: the frame of make_counter has returned. GCC 15 and Clang 23
    # catch this with the default ASAN_OPTIONS; no option is set on purpose,
    # so that the test fails if a default changes.
    add_test(NAME e004_asan_catches_escape-ref COMMAND e004_demo_unsafe escape-ref)
    set_tests_properties(e004_asan_catches_escape-ref PROPERTIES
        PASS_REGULAR_EXPRESSION "AddressSanitizer: stack-use-after-return"
        LABELS "vangnet")
endif()
