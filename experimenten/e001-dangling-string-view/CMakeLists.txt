# e001 — dangling std::string_view
add_library(e001_first_word STATIC first_word.cpp)
leren_target(e001_first_word)
target_include_directories(e001_first_word PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests of the safe use. Must pass under every preset.
leren_gtest(e001_test first_word_test.cpp)
target_link_libraries(e001_test PRIVATE e001_first_word)

# The demo has undefined behaviour on purpose.
add_executable(e001_demo_unsafe demo_unsafe.cpp)
leren_target(e001_demo_unsafe)
target_link_libraries(e001_demo_unsafe PRIVATE e001_first_word)

# Under AddressSanitizer the demo must be caught. Each test passes when ASan
# prints the expected report, and fails when the bug slips through.
# Table: demo mode -> report that ASan must print.
set(e001_asan_expect_plain     "AddressSanitizer: heap-use-after-free")
set(e001_asan_expect_annotated "AddressSanitizer: heap-use-after-free")
set(e001_asan_expect_short     "AddressSanitizer: stack-use-after-scope")

if(LEREN_SANITIZE STREQUAL "address")
    foreach(mode IN ITEMS plain annotated short)
        add_test(NAME e001_asan_catches_${mode} COMMAND e001_demo_unsafe ${mode})
        set_tests_properties(e001_asan_catches_${mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "${e001_asan_expect_${mode}}"
            LABELS "vangnet")
    endforeach()
endif()
