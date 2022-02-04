cmake_minimum_required(VERSION 3.16)

project(sfa CXX)

set(CMAKE_CXX_STANDARD 17)

# Make the compiler as pedantic as possible.
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)

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

############################## Linux Unit Tests ################################

file(GLOB utest-linux-src
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
add_executable(utest-linux ${utest-linux-src})
target_link_libraries(utest-linux CppUTest pthread)
target_include_directories(utest-linux PRIVATE . utest)
