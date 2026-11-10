#include "Grid.hpp"
#include "GridIO.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Τυπώνει το πλέγμα σαν πίνακα, με στοιχισμένες στήλες.
void printgrid(const Grid& g) {
    for (int r = 0; r < g.rows(); r++) {
        for (int c = 0; c < g.cols(); c++) {
            std::cout << std::setw(6) << g.at(r, c);
        }
        std::cout << '\n';
    }
}

int main() {
    // 1. Το σωστό αρχείο πρέπει να φορτώνει
    try {
        Grid g = loadGridCsv("data/slope_4x4.csv", 30.0);
        std::cout << "Loaded " << g.rows() << "x" << g.cols()
                  << " grid, cellsize " << g.cellsize() << " m\n";
        printgrid(g);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    // 2. Τα κακά αρχεία πρέπει ΟΛΑ να απορρίπτονται
    std::cout << "\nBad file tests:\n";
    const std::vector<std::string> badFiles = {
        "data/bad_ragged.csv",
        "data/bad_number.csv",
        "data/bad_empty.csv",
        "data/does_not_exist.csv",
    };

    int failures = 0;
    for (const std::string& file : badFiles) {
        try {
            loadGridCsv(file, 30.0);
            std::cout << "BUG " << file << " was accepted\n";
            failures++;
        } catch (const std::runtime_error& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
    }

    // 3. Λάθος cellsize: το πιάνει ο constructor της Grid
    try {
        loadGridCsv("data/slope_4x4.csv", 0.0);
        std::cout << "BUG cellsize 0 was accepted\n";
        failures++;
    } catch (const std::invalid_argument& e) {
        std::cout << "OK  " << e.what() << '\n';
    }

    if (failures > 0) {
        std::cout << '\n' << failures << " test(s) FAILED\n";
        return 1;
    }
    std::cout << "\nAll tests passed\n";
    return 0;
}