# THIS FILE IS USED FOR CLion compatibility
cmake_minimum_required(VERSION 3.16)
project(abstractica C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Get all C files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.c"
        "${CMAKE_SOURCE_DIR}/lib/*.h"
        "${CMAKE_SOURCE_DIR}/lib/*.c"
)

add_executable(abstractica ${SOURCES})

# Include roots
target_include_directories(abstractica PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib
)
