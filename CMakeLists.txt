cmake_minimum_required(VERSION 3.16)

project(sfa CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(./src)

# Collect the PAL unit tests for testing the PSL.
file(GLOB psl-utest-src "src/sfa/pal/utest/*.cpp")

# Figure out which platform-specific code to use for the target platform.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # Use the Linux PSL.
    file(GLOB psl-src "src/sfa/psl/linux/*.cpp")
    link_libraries(pthread)
    # Add in distro-specific code.
    if(SFA_PLATFORM_NILRT)
        # NILRT
        file(GLOB nilrt-utest-src "src/sfa/psl/nilrt/utest/*.cpp")
        list(APPEND psl-utest-src ${nilrt-utest-src})
    else()
        # Some other Linux, probably the build host.
        file(GLOB linux-utest-src "src/sfa/psl/linux/utest/*.cpp")
        list(APPEND psl-utest-src ${linux-utest-src})
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Arduino")
    # Use the Arduino PSL.
    file(GLOB psl-src "src/sfa/psl/arduino/*.cpp")
    # Disable `F64` type since `long double` is 4 bytes on Arduino.
    add_compile_options(-DSFA_NO_F64 -DSFA_PLATFORM_ARDUINO)
else()
    message(FATAL_ERROR "unsupported target platform")
endif()

# Requires that the CppUTest submodule be initialized. If this errors out, run
# `git submodule update --init --recursive`.
add_subdirectory(cpputest)

add_subdirectory(examples)

##################################### CLI ######################################

# Target `cli` builds the SFA CLI tool (for checking config files, running state
# machine tests, etc.).

file(GLOB cli-src "src/sfa/cli/*.cpp")
add_executable(cli ${cli-src})
target_link_libraries(cli PRIVATE sfasup sfa)
# Make the compiler as pedantic as possible.
target_compile_options(cli PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)

################################## Unit Tests ##################################

# Target `utest` builds the full SFA unit test suite. This consists of unit
# tests for the SFA core library, SFA support library, PAL, and PSL for the
# target platform. Currently this means that the platform targeted by `utest`
# must support the SFA support library (see target `sfasup`); creating a more
# granular unit test suite must be done manually.
#
# Note: some unit tests may fail if the process does not have permission to
# create real-time threads (mostly a concern on Linux; fix with `sudo`).

file(GLOB utest-src
    "src/sfa/core/utest/*.cpp"
    "src/sfa/sup/utest/*.cpp"
    "src/sfa/utest/*.cpp"
)
# Add in PSL unit tests for the target platform.
list(APPEND utest-src ${psl-utest-src})
add_executable(utest ${utest-src})
target_link_libraries(utest PRIVATE sfasup sfa CppUTest)

############################## SFA Core Library ################################

# Target `sfa` builds the SFA core static library, the main API layer that SFA
# applications rest on top of. This library also includes the PSL for the target
# platform. This library can stand alone (no other linkages necessary).

file(GLOB sfa-src "src/sfa/core/*.cpp")
# Add in the PSL for the target platform.
list(APPEND sfa-src ${psl-src})
add_library(sfa ${sfa-src})
# Make the compiler as pedantic as possible.
target_compile_options(sfa PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)

# When cross-compiling to Arduino, link libraries needed by the Arduino PSL. If
# using sockets, the user should provide the Arduino's MAC address in the form
# of a 6-byte hex constant (e.g., 0xAABBCCDDEEFF) via a CMake variable named
# `SFA_ARDUINO_MAC_ADDR`.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Arduino")
    target_link_arduino_libraries(sfa PRIVATE core Ethernet)
    target_compile_options(sfa PRIVATE
        -DSFA_ARDUINO_MAC_ADDR=${SFA_ARDUINO_MAC_ADDR}
    )
endif()

############################ SFA Support Library ###############################

# Target `sfasup` builds the SFA support static library. This includes support
# components like config parsers and autocoders. This library requires an STL
# (GCC 4.9.2+) and a heap. This library must be linked alongside the SFA core
# library (see target `sfa`).

file(GLOB sfasup-src "src/sfa/sup/*.cpp")
add_library(sfasup ${sfasup-src})
# Make the compiler as pedantic as possible.
target_compile_options(sfasup PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)
