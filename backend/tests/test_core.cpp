// Unit tests για terrain_core. Τρέχουν με: ctest --test-dir build --output-on-failure
// Πρέπει να τρέχουν από τον φάκελο backend/ (διαβάζουν data/...).
#include "Difficulty.hpp"
#include "Gradient.hpp"
#include "Grid.hpp"
#include "GridIO.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Τυπώνει το πλέγμα σαν πίνακα, με 2 δεκαδικά.
static void printgrid(const Grid& g) {
    for (int r = 0; r < g.rows(); r++) {
        for (int c = 0; c < g.cols(); c++) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                      << g.at(r, c);
        }
        std::cout << '\n';
    }
}

// Ελέγχει ότι ΚΑΘΕ κελί του πλέγματος είναι ίσο με expected (με ανοχή).
// Επιστρέφει πόσα κελιά ήταν λάθος.
static int checkAll(const Grid& g, double expected, double tolerance) {
    int wrong = 0;
    for (int r = 0; r < g.rows(); r++) {
        for (int c = 0; c < g.cols(); c++) {
            if (std::abs(g.at(r, c) - expected) > tolerance) {
                wrong++;
            }
        }
    }
    return wrong;
}

int main() {
    int failures = 0;

    // ---------- 1. Φόρτωση CSV ----------
    std::cout << "== CSV loader ==\n";
    try {
        Grid g = loadGridCsv("data/slope_4x4.csv", 30.0);
        std::cout << "Loaded " << g.rows() << "x" << g.cols()
                  << " grid, cellsize " << g.cellsize() << " m\n";
        printgrid(g);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    const std::vector<std::string> badFiles = {
        "data/bad_ragged.csv",
        "data/bad_number.csv",
        "data/bad_empty.csv",
        "data/bad_nan.csv",
        "data/does_not_exist.csv",
    };
    for (const std::string& file : badFiles) {
        try {
            loadGridCsv(file, 30.0);
            std::cout << "BUG " << file << " was accepted\n";
            failures++;
        } catch (const std::runtime_error& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
    }

    try {
        loadGridCsv("data/slope_4x4.csv", 0.0);
        std::cout << "BUG cellsize 0 was accepted\n";
        failures++;
    } catch (const std::invalid_argument& e) {
        std::cout << "OK  " << e.what() << '\n';
    }

    // ---------- 2. Κλίσεις με γνωστή απάντηση ----------
    std::cout << "\n== Slopes ==\n";

    struct SlopeTest {
        std::string file;
        double expectedDegrees;
    };
    const std::vector<SlopeTest> slopeTests = {
        {"data/flat_4x4.csv", 0.0},       // επίπεδο
        {"data/slope_4x4.csv", 26.565},   // arctan(0.5)
        {"data/diag_4x4.csv", 35.264},    // arctan(sqrt(0.5))
    };

    for (const SlopeTest& t : slopeTests) {
        try {
            Grid slopes = slopeGrid(loadGridCsv(t.file, 30.0));
            std::cout << t.file << " (expected " << t.expectedDegrees << "):\n";
            printgrid(slopes);

            int wrong = checkAll(slopes, t.expectedDegrees, 0.01);
            if (wrong == 0) {
                std::cout << "OK  all cells correct\n\n";
            } else {
                std::cout << "BUG " << wrong << " cell(s) wrong\n\n";
                failures++;
            }
        } catch (const std::exception& e) {
            std::cout << "BUG " << t.file << ": " << e.what() << "\n\n";
            failures++;
        }
    }

    // ---------- 3. Ακραία περίπτωση: πλέγμα με 1 γραμμή ----------
    std::cout << "== Edge case: 1 row ==\n";
    try {
        Grid row(1, 3, 30.0);
        row.set(0, 0, 100.0);
        row.set(0, 1, 115.0);
        row.set(0, 2, 130.0);
        Grid slopes = slopeGrid(row);   // δεν πρέπει να διαβάσει εκτός ορίων
        int wrong = checkAll(slopes, 26.565, 0.01);
        if (wrong == 0) {
            std::cout << "OK  1-row grid handled\n";
        } else {
            std::cout << "BUG 1-row grid: " << wrong << " cell(s) wrong\n";
            failures++;
        }
    } catch (const std::exception& e) {
        std::cout << "BUG 1-row grid threw: " << e.what() << '\n';
        failures++;
    }

    // ---------- 4. Κατηγορίες δυσκολίας ----------
    std::cout << "\n== Difficulty ==\n";

    struct ClassifyTest {
        double degrees;
        Difficulty expected;
    };
    const std::vector<ClassifyTest> classifyTests = {
        {9.99, Difficulty::Easy},       // λίγο κάτω από το όριο 10
        {10.0, Difficulty::Moderate},   // ακριβώς στο όριο
        {19.99, Difficulty::Moderate},
        {20.0, Difficulty::Steep},
        {30.0, Difficulty::VerySteep},
    };
    for (const ClassifyTest& t : classifyTests) {
        try {
            Difficulty got = classify(t.degrees);
            if (got == t.expected) {
                std::cout << "OK  " << t.degrees << " -> " << toString(got) << '\n';
            } else {
                std::cout << "BUG " << t.degrees << " -> " << toString(got)
                          << ", expected " << toString(t.expected) << '\n';
                failures++;
            }
        } catch (const std::exception& e) {
            std::cout << "BUG " << t.degrees << " threw: " << e.what() << '\n';
            failures++;
        }
    }

    // Άκυρες γωνίες: πρέπει να πετάξουν std::invalid_argument
    const std::vector<double> badAngles = {-1.0, 91.0, std::nan("")};
    for (double d : badAngles) {
        try {
            Difficulty got = classify(d);
            std::cout << "BUG " << d << " was accepted as " << toString(got) << '\n';
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
    }

    // Χάρτης δυσκολίας για το slope_4x4.csv
    std::cout << "\nDifficulty map for data/slope_4x4.csv:\n";
    try {
        Grid slopes = slopeGrid(loadGridCsv("data/slope_4x4.csv", 30.0));
        for (int r = 0; r < slopes.rows(); r++) {
            for (int c = 0; c < slopes.cols(); c++) {
                std::cout << std::setw(12) << toString(classify(slopes.at(r, c)));
            }
            std::cout << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "BUG difficulty map: " << e.what() << '\n';
        failures++;
    }

    // ---------- Αποτέλεσμα ----------
    if (failures > 0) {
        std::cout << '\n' << failures << " test(s) FAILED\n";
        return 1;
    }
    std::cout << "\nAll tests passed\n";
    return 0;
}