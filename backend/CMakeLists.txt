cmake_minimum_required(VERSION 3.16)
project(hiking-terrain VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(terrain src/main.cpp src/Grid.cpp src/GridIO.cpp)

target_compile_options(terrain PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
target_include_directories(terrain PRIVATE include)