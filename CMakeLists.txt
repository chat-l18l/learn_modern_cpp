# Top-level build for the C++ learning experiments.
# Configure through a preset: see CMakePresets.json and `pixi run test <preset>`.
cmake_minimum_required(VERSION 3.28)
project(cpp_leren LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# No C++ modules in this repo (yet). Without this line CMake 4 builds the std
# module for every target, which costs time and floods the log with warnings.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Which runtime checker to build in. The presets set this value.
#   none     no sanitizer
#   address  AddressSanitizer + UndefinedBehaviorSanitizer
#   thread   ThreadSanitizer
set(LEREN_SANITIZE "none" CACHE STRING "Sanitizer: none, address or thread")
set_property(CACHE LEREN_SANITIZE PROPERTY STRINGS none address thread)

# Audit mode (decision B14): treat code of others in vreemd/ as our own code,
# with all warnings, and add -Wunsafe-buffer-usage (Clang only).
option(LEREN_AUDIT "Audit code of others: no -isystem, extra warnings" OFF)

include(cmake/leren.cmake)

find_package(GTest 1.14 REQUIRED CONFIG)
include(GoogleTest)
enable_testing()

add_subdirectory(vreemd)
add_subdirectory(experimenten)
