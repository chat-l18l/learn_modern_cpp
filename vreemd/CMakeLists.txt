# Code of others. See vreemd/README.md.
#
# Normal presets treat minilib like an imported package: headers through
# -isystem and no warnings on its sources. The audit preset treats it like our
# own code, with all warnings, but without -Werror: its warnings are findings.

set(minilib_dir ${CMAKE_CURRENT_SOURCE_DIR}/minilib)

# minilib: built from source with the sanitizer of the active preset.
add_library(minilib STATIC ${minilib_dir}/minilib.cpp)
# minilib_prebuilt: stands in for a binary package. Never instrumented.
add_library(minilib_prebuilt STATIC ${minilib_dir}/minilib.cpp)

if(LEREN_AUDIT)
    leren_target(minilib UNSAFE)
    target_include_directories(minilib PUBLIC ${minilib_dir})
else()
    target_compile_options(minilib PRIVATE ${LEREN_SAN_FLAGS})
    target_include_directories(minilib SYSTEM PUBLIC ${minilib_dir})
endif()
target_include_directories(minilib_prebuilt SYSTEM PUBLIC ${minilib_dir})
