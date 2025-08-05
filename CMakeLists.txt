cmake_minimum_required(VERSION 3.15)

project(int128)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE .)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE int128.cc)
