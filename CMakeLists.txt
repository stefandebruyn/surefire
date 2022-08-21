cmake_minimum_required(VERSION 3.16)

project(sf C CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(./src)

# Collect the PAL unit tests for testing the PSL.
file(GLOB psl-utest-src "src/sf/pal/utest/*.cpp")

# Figure out which platform-specific code to use for the target platform.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_options(-DSF_PLATFORM_LINUX)
    # Use the Linux PSL.
    file(GLOB psl-src "src/sf/psl/linux/*.cpp")
    link_libraries(pthread)
    # Add in distro-specific code.
    if(SF_PLATFORM_SBRIO9637)
        # sbRIO-9637
        file(GLOB nilrt-utest-src "src/sf/psl/sbrio9637/utest/*.cpp")
        list(APPEND psl-utest-src ${nilrt-utest-src})
        file (GLOB nilrt-psl-src
            "src/sf/psl/sbrio9637/*.cpp"
            "src/sf/psl/sbrio9637/nifpga/*.c"
        )
        list(APPEND psl-src ${nilrt-psl-src})
    else()
        # Some other Linux, probably the build host.
        file(GLOB linux-utest-src "src/sf/psl/linux/utest/*.cpp")
        list(APPEND psl-utest-src ${linux-utest-src})
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Arduino")
    # Use the Arduino PSL.
    file(GLOB psl-src "src/sf/psl/arduino/*.cpp")
    # Allow F64 type to not be 8 bytes since most Arduinos lack true doubles.
    add_compile_options(-DSF_ALLOW_MISSIZED_F64 -DSF_PLATFORM_ARDUINO)
else()
    message(FATAL_ERROR "unsupported target platform")
endif()

# Enable framework unsafe asserts if specified.
if(${SF_ENABLE_ASSERTS})
    add_compile_options(-DSF_ENABLE_ASSERTS)
endif()

# Enable saving of safe assert fail sites if specified.
if(${SF_SAFE_ASSERT_SAVES_FAIL_SITE})
    add_compile_options(-DSF_SAFE_ASSERT_SAVES_FAIL_SITE)
endif()

# Define a symbol for the repository path. This is used by some unit tests that
# access files elsewhere in the repository, e.g., autocoder tests.
add_compile_options(-DSF_REPO_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# Enable code coverage if specified.
if(${SF_COVERAGE})
    add_compile_options(--coverage)
    link_libraries(gcov)
endif()

add_subdirectory(examples)

##################################### CLI ######################################

# Target `cli` builds the Surefire CLI tool (for checking config files, running
# state machine tests, etc.). The executable is named `sf`.

file(GLOB cli-src "src/sf/cli/*.cpp")
add_executable(cli ${cli-src})
set_target_properties(cli PROPERTIES OUTPUT_NAME "sf")
target_link_libraries(cli PRIVATE sfconfig sf)
# Make the compiler as pedantic as possible.
target_compile_options(cli PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-comment # Otherwise ASCII diagrams with backslashes will error out.
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
)

################################## Unit Tests ##################################

# Target `utest` builds the full Surefire unit test suite. This consists of unit
# tests for the Surefire core library, config library, PAL, and PSL for the
# target platform. Currently this means that the platform targeted by `utest`
# must support the Surefire config library (see target `sfconfig`); creating a
# more granular unit test suite must be done manually.
#
# Note: some unit tests may fail if the process does not have permission to
# create real-time threads (mostly a concern on Linux; fix with sudo).

file(GLOB utest-src
    "src/sf/core/utest/*.cpp"
    "src/sf/config/utest/*.cpp"
    "src/sf/utest/*.cpp"
)
# Add in PSL unit tests for the target platform.
list(APPEND utest-src ${psl-utest-src})
add_executable(utest ${utest-src})
target_link_libraries(utest PRIVATE sfconfig sf CppUTest)

############################ Surefire Core Library #############################

# Target `sf` builds the Surefire core static library, the main API layer that
# Surefire applications rest on top of. This library also includes the PSL for
# the target platform. This library can stand alone.

file(GLOB sf-src "src/sf/core/*.cpp")
# Add in the PSL for the target platform.
list(APPEND sf-src ${psl-src})
add_library(sf ${sf-src})
# Make the compiler as pedantic as possible.
target_compile_options(sf PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-comment
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
)

# When cross-compiling to Arduino, link libraries needed by the Arduino PSL. If
# using sockets, the user should provide the Arduino's MAC address in the form
# of a 6-byte hex constant (e.g., "0xAABBCCDDEEFF") via a CMake variable named
# `SF_ARDUINO_MAC_ADDR`.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Arduino")
    target_link_arduino_libraries(sf PRIVATE core Ethernet)
    target_compile_options(sf PRIVATE
        -DSF_ARDUINO_MAC_ADDR=${SF_ARDUINO_MAC_ADDR}
    )
endif()

########################### Surefire Config Library ############################

# Target `sfconfig` builds the Surefire config static library. This includes
# components like config parsers and autocoders. This library requires a C++
# Standard Library (GCC 4.9.2+) and a heap. This library must be linked
# alongside the Surefire core library (see target `sf`).

file(GLOB sfconfig-src "src/sf/config/*.cpp")
add_library(sfconfig ${sfconfig-src})
# Make the compiler as pedantic as possible.
target_compile_options(sfconfig PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-comment
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
)
