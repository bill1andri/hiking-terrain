cmake_minimum_required(VERSION 3.16)
project(hiking-terrain VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- Βιβλιοθήκη: όλη η λογική (χωρίς main) ----------
add_library(terrain_core STATIC
    src/Grid.cpp
    src/GridIO.cpp
    src/Gradient.cpp
    src/Difficulty.cpp
)
# PUBLIC: όποιος κάνει link στο terrain_core βλέπει αυτόματα το include/
target_include_directories(terrain_core PUBLIC include)
target_compile_options(terrain_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# ---------- Εκτελέσιμο: terrain <file.csv> <cellsize> ----------
add_executable(terrain src/main.cpp)
target_link_libraries(terrain PRIVATE terrain_core)
target_compile_options(terrain PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# ---------- Tests (τρέχουν με ctest) ----------
enable_testing()

add_executable(tests tests/test_core.cpp)
target_link_libraries(tests PRIVATE terrain_core)
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Wshadow)

# Τα tests ανοίγουν data/..., άρα τρέχουν από τον φάκελο backend/
add_test(NAME core_tests
         COMMAND tests
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Το CLI πρέπει να πετυχαίνει σε καλό αρχείο...
add_test(NAME cli_diag
         COMMAND terrain data/diag_4x4.csv 30
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ...και να αποτυγχάνει (exit code != 0) σε κακή είσοδο
add_test(NAME cli_bad_file
         COMMAND terrain data/bad_nan.csv 30
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME cli_bad_cellsize
         COMMAND terrain data/diag_4x4.csv abc
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME cli_no_args
         COMMAND terrain
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(cli_bad_file cli_bad_cellsize cli_no_args
                     PROPERTIES WILL_FAIL TRUE)
