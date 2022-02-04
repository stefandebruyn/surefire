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
