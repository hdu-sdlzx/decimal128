cmake_minimum_required(VERSION 3.15)

project(int128)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE .)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_20)

if (MSVC)
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /Zc:__cplusplus /utf-8)
endif()
