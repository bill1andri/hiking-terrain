cmake_minimum_required(VERSION 3.16)
project(hiking-weather VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(weather src/main.cpp)
target_compile_options(weather PRIVATE -Wall -Wextra -Wpedantic -Wshadow)