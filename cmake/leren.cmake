# Shared settings for every experiment target.
#
# leren_target(<target> [UNSAFE] [HOST]) gives a target the warning set and
# the sanitizer flags of the active preset. Every target in this repo calls it.
#
# Without UNSAFE the target builds with -Werror: safe code must have no
# warnings (decision B12). The audit preset never uses -Werror. With UNSAFE, warnings are measurements and do not
# stop the build. Only demos with undefined behaviour on purpose, and code of
# others under audit, get UNSAFE.
#
# Without HOST the target also has a stack budget per function (decision B16).
# The budget applies only in presets without a sanitizer.
# HOST marks code that only runs on the development machine: the tests. Their
# frames are large for reasons that do not reach a device (GoogleTest
# registers every TEST() in one static initialiser).

set(LEREN_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wold-style-cast
    -Woverloaded-virtual
    -Wnon-virtual-dtor
    -Wimplicit-fallthrough
    -Wformat=2
    -Wundef
    -Wcast-align
    -Wdouble-promotion
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND LEREN_WARNINGS -Wdangling-reference -Wuseless-cast -Wduplicated-cond)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND LEREN_WARNINGS -Wdangling -Wreturn-stack-address -Wlifetime-safety)
endif()

# Stack budget per function, in bytes (decision B16). Every target without
# UNSAFE builds with -Wframe-larger-than and -Werror: a function whose stack
# frame grows past the budget does not compile. Large data does not belong on
# the stack: the caller allocates it and passes a reference or a span.
set(LEREN_FRAME_LIMIT 512 CACHE STRING "Maximum stack frame per function, in bytes")

# Extra warnings for the audit preset. They apply to every target, so that
# our own code meets the same bar as the code we audit.
set(LEREN_EXTRA_WARNINGS "")
if(LEREN_AUDIT)
    list(APPEND LEREN_EXTRA_WARNINGS -Wunsafe-buffer-usage)
endif()

# One table: sanitizer name -> compile and link flags.
set(LEREN_SAN_FLAGS_none "")
set(LEREN_SAN_FLAGS_address
    -fsanitize=address,undefined
    -fno-sanitize-recover=all
    -fno-omit-frame-pointer)
set(LEREN_SAN_FLAGS_thread
    -fsanitize=thread
    -fno-omit-frame-pointer)

if(NOT DEFINED LEREN_SAN_FLAGS_${LEREN_SANITIZE})
    message(FATAL_ERROR "LEREN_SANITIZE='${LEREN_SANITIZE}' is unknown; use none, address or thread")
endif()
set(LEREN_SAN_FLAGS ${LEREN_SAN_FLAGS_${LEREN_SANITIZE}})

include(CheckCXXCompilerFlag)

function(leren_target target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "UNSAFE;HOST" "" "")
    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "leren_target(${target}): unknown arguments ${arg_UNPARSED_ARGUMENTS}")
    endif()
    foreach(flag IN LISTS LEREN_WARNINGS LEREN_EXTRA_WARNINGS)
        string(MAKE_C_IDENTIFIER "HAS${flag}" var)
        check_cxx_compiler_flag(${flag} ${var})
        if(${var})
            target_compile_options(${target} PRIVATE ${flag})
        endif()
    endforeach()
    # The audit preset is a report: it must show every finding, so it never
    # stops at the first error. The other presets enforce -Werror.
    # Only without a sanitizer: ASan adds red zones to every frame, and a device
    # never runs with ASan. Measured in clang-asan: std::vector's growth path
    # took 536 bytes there, and fits in 512 without ASan.
    if(NOT arg_UNSAFE AND NOT arg_HOST AND LEREN_SANITIZE STREQUAL "none")
        target_compile_options(${target} PRIVATE -Wframe-larger-than=${LEREN_FRAME_LIMIT})
    endif()
    if(NOT arg_UNSAFE AND NOT LEREN_AUDIT)
        target_compile_options(${target} PRIVATE -Werror)
    endif()
    target_compile_options(${target} PRIVATE ${LEREN_SAN_FLAGS})
    target_link_options(${target} PRIVATE ${LEREN_SAN_FLAGS})
endfunction()

# leren_gtest(<target> <sources>...) builds a GoogleTest binary and registers
# each TEST() with CTest.
function(leren_gtest target)
    add_executable(${target} ${ARGN})
    leren_target(${target} HOST)
    target_link_libraries(${target} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST)
endfunction()

# leren_compile_fail_test(<name> <source> <regex>) registers a test that
# compiles <source> (relative to the current source dir) and passes only when
# the compiler REFUSES it with a message that matches <regex>. Use it to prove
# that a type rejects a mistake that a concept cannot express, such as the
# order of designated initializers. The flags are fixed and include -Werror,
# as for safe code (B12): some guards exist only as warnings in one compiler.
function(leren_compile_fail_test name source regex)
    add_test(NAME ${name}
        COMMAND ${CMAKE_CXX_COMPILER} -std=c++23 -fsyntax-only
                -Wall -Wextra -Wpedantic -Werror
                -I${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/${source})
    set_tests_properties(${name} PROPERTIES
        PASS_REGULAR_EXPRESSION "${regex}"
        LABELS "compile-fail")
endfunction()
