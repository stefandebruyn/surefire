cmake_minimum_required(VERSION 3.16)

project(arduino-example CXX)

add_executable(arduino-example "main.cpp")

# Link Arduino libraries needed by the Surefire Arduino PSL (along with any
# other Arduino libraries you want; if using 3rd party libraries like for
# sensors, see example CMake project in the Arduino CMake Toolchain repo for
# linking 3rd party libraries).
target_link_arduino_libraries(arduino-example PRIVATE core Ethernet)

# Link Surefire core library.
target_link_libraries(arduino-example PRIVATE sf)

# Automatically adds a target `upload-<target name>` (in this case
# `upload-arduino-example`) for uploading to the target board.
target_enable_arduino_upload(arduino-example)
