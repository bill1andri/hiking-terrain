cmake_minimum_required(VERSION 3.16)
project(hiking-weather VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library without the logic(no main)
add_library(weather_core STATIC src/Atmosphere.cpp)
target_include_directories(weather_core PUBLIC include)
target_compile_options(weather_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# Executable
add_executable(weather src/main.cpp)
target_link_libraries(weather PRIVATE weather_core)
target_compile_options(weather PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# Tests
enable_testing()

add_executable(test_atmosphere tests/test_atmosphere.cpp)
target_link_libraries(test_atmosphere PRIVATE weather_core)
target_compile_options(test_atmosphere PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
add_test(NAME atmosphere_tests COMMAND test_atmosphere)