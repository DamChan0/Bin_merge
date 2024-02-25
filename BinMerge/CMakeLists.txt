cmake_minimum_required(VERSION 3.16.3)

project(BinMerge)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(BinMerge SHARED
    BinMerge.cc
    fileControl.cc
)

target_include_directories(BinMerge
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    *.h
)

set(CMAKE_BUILD_TYPE Debug)

target_compile_options(BinMerge
    PRIVATE
    -g -Wall -Wextra -Werror
    -Wno-error=missing-field-initializers
    -Wno-error=write-strings
    PUBLIC
    -D_FILE_OFFSET_BITS=64
)

if(EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
    configure_file(
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
        COPYONLY)
endif()
