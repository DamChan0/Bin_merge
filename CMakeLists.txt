cmake_minimum_required(VERSION 3.16.3)

project(MergeLidar)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(BinMerge)

add_subdirectory(Qt)
add_executable(${PROJECT_NAME} main.cc)

target_link_libraries("${PROJECT_NAME}" PUBLIC BinMerge mergebin_qt)
