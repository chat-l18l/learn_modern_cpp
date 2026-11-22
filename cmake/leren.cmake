# Shared settings for every experiment target.
#
# leren_target(<target>) gives a target the warning set and the sanitizer
# flags of the active preset. Every target in this repo calls it.

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
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND LEREN_WARNINGS -Wdangling-reference -Wuseless-cast -Wduplicated-cond)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND LEREN_WARNINGS -Wdangling -Wreturn-stack-address -Wlifetime-safety)
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
    foreach(flag IN LISTS LEREN_WARNINGS)
        string(MAKE_C_IDENTIFIER "HAS${flag}" var)
        check_cxx_compiler_flag(${flag} ${var})
        if(${var})
            target_compile_options(${target} PRIVATE ${flag})
        endif()
    endforeach()
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
