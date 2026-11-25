cmake_minimum_required(VERSION 3.16)
project(hiking-weather VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Static core library containing atmospheric and safety rule logic (without main).
add_library(weather_core STATIC src/Atmosphere.cpp src/SafetyRules.cpp src/OpenMeteoClient.cpp src/Cache.cpp)
target_include_directories(weather_core PUBLIC include)
target_include_directories(weather_core SYSTEM PRIVATE third_party)
target_link_libraries(weather_core PUBLIC OpenSSL::SSL OpenSSL::Crypto Threads::Threads)
target_compile_options(weather_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# Main CLI executable linked against weather_core.
add_executable(weather src/main.cpp)
target_link_libraries(weather PRIVATE weather_core)
target_compile_options(weather PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# Unit test executables registered with CTest.
enable_testing()

add_executable(test_atmosphere tests/test_atmosphere.cpp)
target_link_libraries(test_atmosphere PRIVATE weather_core)
target_compile_options(test_atmosphere PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
add_test(NAME atmosphere_tests COMMAND test_atmosphere)

add_executable(test_rules tests/test_rules.cpp)
target_link_libraries(test_rules PRIVATE weather_core)
target_compile_options(test_rules PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
add_test(NAME rules_tests COMMAND test_rules)

add_executable(test_parse tests/test_parse.cpp)
target_link_libraries(test_parse PRIVATE weather_core)
target_compile_options(test_parse PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
add_test(NAME parse_tests COMMAND test_parse WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})