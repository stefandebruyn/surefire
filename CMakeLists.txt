cmake_minimum_required(VERSION 3.16)

project(sfa CXX)

set(CMAKE_CXX_STANDARD 11)

# Requires that the CppUTest submodule be initialized. If this errors out, run
# `git submodule update --init --recursive`.
add_subdirectory(cpputest)

##################################### CLI ######################################

# `make cli` builds the SFA CLI tool (for checking config files, running state
# machine tests, etc.).

file(GLOB cli-src "src/sfa/cli/*.cpp")
add_executable(cli ${cli-src})
target_link_libraries(cli sfasup sfa)
target_include_directories(cli PRIVATE ./src)

# Make the compiler as pedantic as possible.
target_compile_options(cli PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)

################################## Unit Tests ##################################

# `make utest` builds the full SFA unit test suite. This consists of unit tests
# for the SFA core library, SFA support library, and PAL.

file(GLOB utest-src
    "src/sfa/core/utest/*.cpp"
    "src/sfa/pal/utest/*.cpp"
    "src/sfa/sup/utest/*.cpp"
    "src/sfa/utest/*.cpp"
)
add_executable(utest ${utest-src})
target_link_libraries(utest sfasup sfa CppUTest)
target_include_directories(utest PRIVATE ./src ./src/sfa/utest)

############################## SFA Core Library ################################

# `make sfa` builds the SFA core static library. This includes the standalone
# SFA components (that don't need an OS, heap, etc.), the PAL, and a PSL for
# the target platform.

file(GLOB sfa-src
    "src/sfa/core/*.cpp"
    "src/sfa/psl/linux/*.cpp"
)
add_library(sfa ${sfa-src})
target_link_libraries(sfa pthread)
target_include_directories(sfa PRIVATE ./src)

# Make the compiler as pedantic as possible.
target_compile_options(sfa PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)

############################ SFA Support Library ###############################

# `make sfasup` builds the SFA support static library. This includes support
# components like config parsers and autocoders. This library requires an STL
# (GCC 4.9.2+) and a heap.

file(GLOB sfasup-src
    "src/sfa/sup/*.cpp"
)
add_library(sfasup ${sfasup-src})
target_include_directories(sfasup PRIVATE ./src)

# Make the compiler as pedantic as possible.
target_compile_options(sfasup PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wold-style-cast
)
