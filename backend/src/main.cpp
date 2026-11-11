// terrain: διαβάζει ένα CSV υψομέτρων και τυπώνει κλίσεις και δυσκολία.
//
// Χρήση:   terrain <file.csv> <cellsize_in_meters>
// Π.χ.:    ./build/terrain data/diag_4x4.csv 30
//
// Exit code: 0 = όλα καλά, 1 = λάθος είσοδος (μήνυμα στο stderr).

#include "Difficulty.hpp"
#include "Gradient.hpp"
#include "Grid.hpp"
#include "GridIO.hpp"

#include <array>
#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

// Τυπώνει το πλέγμα σαν πίνακα, με 2 δεκαδικά.
static void printGrid(const Grid& g) {
    for (int r = 0; r < g.rows(); r++) {
        for (int c = 0; c < g.cols(); c++) {
            std::cout << std::setw(8) << std::fixed << std::setprecision(2)
                      << g.at(r, c);
        }
        std::cout << '\n';
    }
}

// Αυστηρή μετατροπή του cellsize: "30" ok, "30m" / "abc" / "" όχι.
static double parseCellsize(const std::string& s) {
    std::size_t pos = 0;
    double v = 0.0;
    try {
        v = std::stod(s, &pos);
    } catch (const std::exception&) {
        throw std::invalid_argument("cellsize is not a number: '" + s + "'");
    }
    if (pos != s.size()) {
        throw std::invalid_argument("cellsize is not a number: '" + s + "'");
    }
    return v;  // το Grid ελέγχει ότι είναι > 0
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file.csv> <cellsize_in_meters>\n"
                  << "Example: " << argv[0] << " data/diag_4x4.csv 30\n";
        return 1;
    }

    try {
        const std::string path = argv[1];
        const double cellsize = parseCellsize(argv[2]);

        Grid heights = loadGridCsv(path, cellsize);
        std::cout << "Loaded " << path << ": " << heights.rows() << "x"
                  << heights.cols() << " grid, cellsize " << cellsize << " m\n";

        std::cout << "\nHeights (m):\n";
        printGrid(heights);

        Grid slopes = slopeGrid(heights);
        std::cout << "\nSlopes (degrees):\n";
        printGrid(slopes);

        // Χάρτης δυσκολίας + μέτρημα ανά κατηγορία
        std::array<int, 4> counts{};  // Easy, Moderate, Steep, VerySteep
        std::cout << "\nDifficulty:\n";
        for (int r = 0; r < slopes.rows(); r++) {
            for (int c = 0; c < slopes.cols(); c++) {
                Difficulty d = classify(slopes.at(r, c));
                counts[static_cast<int>(d)]++;
                std::cout << std::setw(12) << toString(d);
            }
            std::cout << '\n';
        }

        std::cout << "\nSummary:\n";
        const Difficulty all[] = {Difficulty::Easy, Difficulty::Moderate,
                                  Difficulty::Steep, Difficulty::VerySteep};
        for (Difficulty d : all) {
            std::cout << "  " << std::left << std::setw(12) << toString(d)
                      << std::right << counts[static_cast<int>(d)] << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
