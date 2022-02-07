cmake_minimum_required(VERSION 3.16)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Arduino")
    add_subdirectory(arduino)
endif()
