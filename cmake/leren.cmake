# Shared settings for every experiment target.
#
# leren_target(<target> [UNSAFE]) gives a target the warning set and the
# sanitizer flags of the active preset. Every target in this repo calls it.
#
# Without UNSAFE the target builds with -Werror: safe code must have no
# warnings (decision B12). The audit preset never uses -Werror. With UNSAFE, warnings are measurements and do not
# stop the build. Only demos with undefined behaviour on purpose, and code of
# others under audit, get UNSAFE.

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
    cmake_parse_arguments(PARSE_ARGV 1 arg "UNSAFE" "" "")
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
    leren_target(${target})
    target_link_libraries(${target} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST)
endfunction()
