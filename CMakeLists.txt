cmake_minimum_required(VERSION 3.16)

project(sfa CXX)

set(CMAKE_CXX_STANDARD 14)

# Make the compiler as pedantic as possible.
# FIXME: Add these back in for a non-utest library target.
# add_compile_options(
#     -Wall
#     -Wextra
#     -Werror
#     -Wold-style-cast
# )

add_subdirectory(cpputest)

##################################### CLI ######################################

file(GLOB cli-src
    "sfa/cli/*.cpp"
    "sfa/sv/*.cpp"
    "sfa/sm/*.cpp"
    "sfa/config/*.cpp"
    "sfa/util/*.cpp"
)

# Add `Console` implementation appropriate for the build host.
if(UNIX AND NOT APPLE)
    # Linux
    list(APPEND cli-src "psl/linux/Console.cpp")
else()
    message(FATAL_ERROR "unsupported platform")
endif()

add_executable(cli ${cli-src})
target_include_directories(cli PRIVATE .)

################################## Unit Tests ##################################

file(GLOB utest-src
    "sfa/sv/*.cpp"
    "sfa/sv/utest/*.cpp"
    "sfa/sm/*.cpp"
    "sfa/sm/utest/*.cpp"
    "sfa/task/*.cpp"
    "sfa/task/utest/*.cpp"
    "sfa/config/*.cpp"
    "sfa/config/utest/*.cpp"
    "sfa/util/*.cpp"
    "sfa/util/utest/*.cpp"
    "psl/linux/*.cpp"
    "pal/utest/*.cpp"
    "utest/*.cpp"
)

if(SFA_TARGET_PLATFORM_SBRIO9637)
    # sbRIO-9637 uses different priority ranges for `Thread`.
    list(FILTER utest-src EXCLUDE REGEX ".*ThreadPriorityRanges.cpp")
    list(APPEND utest-src "psl/sbrio9637/ThreadPriorityRanges.cpp")
endif()

add_executable(utest ${utest-src})
target_link_libraries(utest CppUTest pthread)
target_include_directories(utest PRIVATE . utest)
