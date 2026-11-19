// Unit tests για terrain_core. Τρέχουν με: ctest --test-dir build --output-on-failure
// Πρέπει να τρέχουν από τον φάκελο backend/ (διαβάζουν data/...).
#include "Crop.hpp"
#include "DemLoader.hpp"
#include "Difficulty.hpp"
#include "GeoUtils.hpp"
#include "Gradient.hpp"
#include "Grid.hpp"
#include "GridIO.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
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

// Σύγκριση doubles με ανοχή (ποτέ ==).
static bool isClose(double a, double b, double tolerance) {
    return std::abs(a - b) < tolerance;
}

int main() {
    int failures = 0;

    // ---------- 1. Φόρτωση CSV ----------
    std::cout << "== CSV loader ==\n";
    try {
        Grid g = loadGridCsv("data/slope_4x4.csv", 30.0);
        std::cout << "Loaded " << g.rows() << "x" << g.cols()
                  << " grid, dx " << g.dx() << " m, dy " << g.dy() << " m\n";
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

    // Μη πεπερασμένο cellsize (nan, inf): πρέπει να πετάξει std::invalid_argument
    const std::vector<double> badCellsizes = {NAN, INFINITY};
    for (double cs : badCellsizes) {
        try {
            Grid g(3, 4, cs);
            std::cout << "BUG cellsize " << cs << " was accepted\n";
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
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

    // ---------- 3b. Μη τετράγωνα κελιά (dx != dy) ----------
    std::cout << "\n== Non-square cells (dx=20, dy=40) ==\n";

    // h = 20*c: ανεβαίνει 20 m ανά στήλη, στήλες απέχουν dx=20 -> tan=1 -> 45°
    // (αν το partialX διαιρούσε με dy θα έβγαινε 26.57°)
    // h = 40*r: ανεβαίνει 40 m ανά γραμμή, γραμμές απέχουν dy=40 -> tan=1 -> 45°
    // (αν το partialY διαιρούσε με dx θα έβγαινε 63.43°)
    struct RectTest {
        std::string name;
        double perCol;  // h += perCol * c
        double perRow;  // h += perRow * r
    };
    const std::vector<RectTest> rectTests = {
        {"h = 20*c", 20.0, 0.0},
        {"h = 40*r", 0.0, 40.0},
    };
    for (const RectTest& t : rectTests) {
        try {
            Grid g(5, 5, 20.0, 40.0);
            for (int r = 0; r < g.rows(); r++) {
                for (int c = 0; c < g.cols(); c++) {
                    g.set(r, c, t.perCol * c + t.perRow * r);
                }
            }
            Grid slopes = slopeGrid(g);
            int wrong = checkAll(slopes, 45.0, 0.01);
            if (wrong == 0) {
                std::cout << "OK  " << t.name << ": all cells 45 degrees\n";
            } else {
                std::cout << "BUG " << t.name << ": " << wrong
                          << " cell(s) wrong, expected 45 degrees\n";
                printgrid(slopes);
                failures++;
            }
        } catch (const std::exception& e) {
            std::cout << "BUG " << t.name << " threw: " << e.what() << '\n';
            failures++;
        }
    }

    // Άκυρα dx / dy: πρέπει να πετάξουν std::invalid_argument
    struct BadSpacing {
        double dx;
        double dy;
    };
    const std::vector<BadSpacing> badSpacings = {
        {0.0, 10.0}, {10.0, -1.0}, {NAN, 10.0}, {10.0, INFINITY},
    };
    for (const BadSpacing& b : badSpacings) {
        try {
            Grid g(3, 3, b.dx, b.dy);
            std::cout << "BUG Grid(3,3," << b.dx << "," << b.dy << ") was accepted\n";
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
    }

    // Constructor με ένα cellsize: dx = dy = cellsize
    try {
        Grid sq(3, 3, 30.0);
        if (sq.dx() == 30.0 && sq.dy() == 30.0) {
            std::cout << "OK  Grid(3,3,30) -> dx=30 dy=30\n";
        } else {
            std::cout << "BUG Grid(3,3,30) -> dx=" << sq.dx() << " dy=" << sq.dy()
                      << ", expected 30 / 30\n";
            failures++;
        }
    } catch (const std::exception& e) {
        std::cout << "BUG Grid(3,3,30) threw: " << e.what() << '\n';
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

    // ---------- 5. GeoUtils ----------
    std::cout << "\n== GeoUtils ==\n";
    const double tolM = 0.01;     // ανοχή για μέτρα
    const double tolDeg = 0.001;  // ανοχή για μοίρες

    // 5a. cellSizeAt με γνωστές απαντήσεις
    try {
        CellSize eq = cellSizeAt(0.0, 3600);
        if (isClose(eq.dx, 30.89, tolM) && isClose(eq.dy, 30.89, tolM)) {
            std::cout << "OK  cellSizeAt(0, 3600) dx=" << eq.dx << " dy=" << eq.dy << '\n';
        } else {
            std::cout << "BUG cellSizeAt(0, 3600) dx=" << eq.dx << " dy=" << eq.dy
                      << ", expected 30.89 / 30.89\n";
            failures++;
        }

        CellSize s60 = cellSizeAt(60.0, 3600);
        if (isClose(s60.dx, s60.dy / 2.0, tolM)) {
            std::cout << "OK  cellSizeAt(60, 3600) dx=" << s60.dx << " = dy/2\n";
        } else {
            std::cout << "BUG cellSizeAt(60, 3600) dx=" << s60.dx << ", expected dy/2="
                      << s60.dy / 2.0 << '\n';
            failures++;
        }

        CellSize ath = cellSizeAt(37.94, 3600);
        if (isClose(ath.dx, 24.36, tolM) && isClose(ath.dy, 30.89, tolM)) {
            std::cout << "OK  cellSizeAt(37.94, 3600) dx=" << ath.dx << " dy=" << ath.dy << '\n';
        } else {
            std::cout << "BUG cellSizeAt(37.94, 3600) dx=" << ath.dx << " dy=" << ath.dy
                      << ", expected 24.36 / 30.89\n";
            failures++;
        }
    } catch (const std::exception& e) {
        std::cout << "BUG cellSizeAt threw: " << e.what() << '\n';
        failures++;
    }

    // 5b. latLonToIndex / indexToLatLon σε ψεύτικο tile (όχι το πραγματικό .hgt)
    try {
        DemTile tile(3601, 37.0, 23.0, std::vector<std::int16_t>(3601 * 3601, 0));

        TileIndex idx = latLonToIndex(tile, 37.9464, 23.8144);
        if (idx.r == 193 && idx.c == 2932) {
            std::cout << "OK  latLonToIndex(37.9464, 23.8144) -> (193, 2932)\n";
        } else {
            std::cout << "BUG latLonToIndex(37.9464, 23.8144) -> (" << idx.r << ", "
                      << idx.c << "), expected (193, 2932)\n";
            failures++;
        }

        LatLon ll = indexToLatLon(tile, 193, 2932);
        if (isClose(ll.lat, 37.9464, tolDeg) && isClose(ll.lon, 23.8144, tolDeg)) {
            std::cout << "OK  indexToLatLon(193, 2932) -> (" << std::setprecision(4)
                      << ll.lat << ", " << ll.lon << ")\n" << std::setprecision(2);
        } else {
            std::cout << "BUG indexToLatLon(193, 2932) -> (" << std::setprecision(4)
                      << ll.lat << ", " << ll.lon << "), expected (37.9464, 23.8144)\n"
                      << std::setprecision(2);
            failures++;
        }

        // Γωνίες: βορειοδυτική = (0,0), νοτιοανατολική = (3600,3600)
        TileIndex nw = latLonToIndex(tile, 38.0, 23.0);
        TileIndex se = latLonToIndex(tile, 37.0, 24.0);
        if (nw.r == 0 && nw.c == 0 && se.r == 3600 && se.c == 3600) {
            std::cout << "OK  corners (38,23) -> (0,0), (37,24) -> (3600,3600)\n";
        } else {
            std::cout << "BUG corners (38,23) -> (" << nw.r << "," << nw.c
                      << "), (37,24) -> (" << se.r << "," << se.c
                      << "), expected (0,0) and (3600,3600)\n";
            failures++;
        }

        // Άκυρες συντεταγμένες: std::invalid_argument
        try {
            latLonToIndex(tile, NAN, 23.5);
            std::cout << "BUG latLonToIndex(nan, 23.5) was accepted\n";
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }

        // Έγκυρες συντεταγμένες αλλά έξω από το tile: std::out_of_range
        try {
            latLonToIndex(tile, 38.17, 23.7);
            std::cout << "BUG latLonToIndex(38.17, 23.7) outside tile was accepted\n";
            failures++;
        } catch (const std::out_of_range& e) {
            std::cout << "OK  " << e.what() << '\n';
        }

        // Δείκτες έξω από το tile: std::out_of_range
        struct BadIndex {
            int r;
            int c;
        };
        const std::vector<BadIndex> badIndices = {{-1, 0}, {0, 3601}};
        for (const BadIndex& b : badIndices) {
            try {
                indexToLatLon(tile, b.r, b.c);
                std::cout << "BUG indexToLatLon(" << b.r << ", " << b.c
                          << ") was accepted\n";
                failures++;
            } catch (const std::out_of_range& e) {
                std::cout << "OK  " << e.what() << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cout << "BUG GeoUtils tile tests threw: " << e.what() << '\n';
        failures++;
    }

    // 5c. cellSizeAt με άκυρη είσοδο: std::invalid_argument
    try {
        cellSizeAt(95.0, 3600);
        std::cout << "BUG cellSizeAt(95, 3600) was accepted\n";
        failures++;
    } catch (const std::invalid_argument& e) {
        std::cout << "OK  " << e.what() << '\n';
    }

    try {
        cellSizeAt(37.94, 0);
        std::cout << "BUG cellSizeAt(37.94, 0) was accepted\n";
        failures++;
    } catch (const std::invalid_argument& e) {
        std::cout << "OK  " << e.what() << '\n';
    }

    // ---------- 6. cropToGrid ----------
    std::cout << "\n== cropToGrid ==\n";

    // Παράθυρο στον Υμηττό (ψεύτικα tiles, όχι το πραγματικό .hgt)
    const double latMin = 37.9233;
    const double latMax = 37.9567;
    const double lonMin = 23.8033;
    const double lonMax = 23.8367;
    const int n = 3601;
    const std::size_t nn = static_cast<std::size_t>(n) * n;

    // 6a. Διαστάσεις και μέγεθος κελιού (tile με μηδενικά)
    try {
        DemTile tile(n, 37.0, 23.0, std::vector<std::int16_t>(nn, 0));
        Grid g = cropToGrid(tile, latMin, latMax, lonMin, lonMax);
        if (g.rows() == 121 && g.cols() == 121) {
            std::cout << "OK  size " << g.rows() << "x" << g.cols() << '\n';
        } else {
            std::cout << "BUG size " << g.rows() << "x" << g.cols()
                      << ", expected 121x121\n";
            failures++;
        }
        if (isClose(g.dx(), 24.36, tolM) && isClose(g.dy(), 30.89, tolM)) {
            std::cout << "OK  dx=" << g.dx() << " dy=" << g.dy() << '\n';
        } else {
            std::cout << "BUG dx=" << g.dx() << " dy=" << g.dy()
                      << ", expected 24.36 / 30.89\n";
            failures++;
        }
    } catch (const std::exception& e) {
        std::cout << "BUG cropToGrid size test threw: " << e.what() << '\n';
        failures++;
    }

    // 6b. Τα ύψη αντιγράφονται από τη σωστή θέση: h(r, c) = c
    // Γραμμές βορράς->νότος, στήλες δύση->ανατολή, με offset nw.
    try {
        std::vector<std::int16_t> data(nn);
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                data[static_cast<std::size_t>(r) * n + c] = static_cast<std::int16_t>(c);
            }
        }
        DemTile tile(n, 37.0, 23.0, std::move(data));
        TileIndex nw = latLonToIndex(tile, latMax, lonMin);
        Grid g = cropToGrid(tile, latMin, latMax, lonMin, lonMax);
        const int cols = g.cols();
        const int rows = g.rows();

        // Ύψη = ακέραιοι, άρα ακριβή σε double: σύγκριση ως int με ==
        int got00 = static_cast<int>(g.at(0, 0));
        int got0last = static_cast<int>(g.at(0, cols - 1));
        int gotlast0 = static_cast<int>(g.at(rows - 1, 0));
        if (got00 == nw.c && got0last == nw.c + cols - 1 && gotlast0 == nw.c) {
            std::cout << "OK  h=c: (0,0)=" << got00 << " (0," << cols - 1 << ")="
                      << got0last << " (" << rows - 1 << ",0)=" << gotlast0 << '\n';
        } else {
            std::cout << "BUG h=c: (0,0)=" << got00 << " (0," << cols - 1 << ")="
                      << got0last << " (" << rows - 1 << ",0)=" << gotlast0
                      << ", expected " << nw.c << " / " << nw.c + cols - 1 << " / "
                      << nw.c << '\n';
            failures++;
        }
    } catch (const std::exception& e) {
        std::cout << "BUG cropToGrid offset test threw: " << e.what() << '\n';
        failures++;
    }

    // 6c. Γνωστή κλίση: h = 10*c -> slope = atan(10 / dx) παντού
    // (αποδεικνύει ότι το cropToGrid περνάει το πραγματικό dx, όχι το dy)
    try {
        std::vector<std::int16_t> data(nn);
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                data[static_cast<std::size_t>(r) * n + c] = static_cast<std::int16_t>(10 * c);
            }
        }
        DemTile tile(n, 37.0, 23.0, std::move(data));
        Grid g = cropToGrid(tile, latMin, latMax, lonMin, lonMax);
        Grid slopes = slopeGrid(g);
        double expected = radToDeg(std::atan(10.0 / g.dx()));
        int wrong = checkAll(slopes, expected, 0.01);
        if (wrong == 0) {
            std::cout << "OK  h=10*c: all cells " << expected << " degrees\n";
        } else {
            std::cout << "BUG h=10*c: " << wrong << " cell(s) wrong, expected "
                      << expected << " degrees\n";
            failures++;
        }
    } catch (const std::exception& e) {
        std::cout << "BUG cropToGrid slope test threw: " << e.what() << '\n';
        failures++;
    }

    // 6d. Σφάλματα και voids
    try {
        DemTile tile(n, 37.0, 23.0, std::vector<std::int16_t>(nn, 0));

        // latMin >= latMax: std::invalid_argument
        try {
            cropToGrid(tile, 37.95, 37.95, lonMin, lonMax);
            std::cout << "BUG latMin == latMax was accepted\n";
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }

        // lonMin >= lonMax: std::invalid_argument
        try {
            cropToGrid(tile, latMin, latMax, lonMax, lonMin);
            std::cout << "BUG lonMin > lonMax was accepted\n";
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }

        // Παράθυρο εν μέρει έξω από το tile: std::out_of_range
        try {
            cropToGrid(tile, latMin, 38.2, lonMin, lonMax);
            std::cout << "BUG window outside tile (latMax 38.2) was accepted\n";
            failures++;
        } catch (const std::out_of_range& e) {
            std::cout << "OK  " << e.what() << '\n';
        }

        // Πολύ μικρό παράθυρο (ίδια γραμμή μετά τη στρογγυλοποίηση): std::invalid_argument
        try {
            cropToGrid(tile, 37.95000, 37.95001, lonMin, lonMax);
            std::cout << "BUG window too small was accepted\n";
            failures++;
        } catch (const std::invalid_argument& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "BUG cropToGrid error tests threw: " << e.what() << '\n';
        failures++;
    }

    // Void (-32768) ΜΕΣΑ στο παράθυρο: std::runtime_error
    try {
        DemTile probe(n, 37.0, 23.0, std::vector<std::int16_t>(nn, 0));
        TileIndex v = latLonToIndex(probe, 37.94, 23.82);
        std::vector<std::int16_t> data(nn, 0);
        data[static_cast<std::size_t>(v.r) * n + v.c] = -32768;
        DemTile tile(n, 37.0, 23.0, std::move(data));
        try {
            cropToGrid(tile, latMin, latMax, lonMin, lonMax);
            std::cout << "BUG void inside window was accepted\n";
            failures++;
        } catch (const std::runtime_error& e) {
            std::cout << "OK  " << e.what() << '\n';
        }
    } catch (const std::exception& e) {
        std::cout << "BUG cropToGrid void-inside test threw: " << e.what() << '\n';
        failures++;
    }

    // Void ΕΞΩ από το παράθυρο (0, 0): δεν πρέπει να πετάξει
    try {
        std::vector<std::int16_t> data(nn, 0);
        data[0] = -32768;
        DemTile tile(n, 37.0, 23.0, std::move(data));
        Grid g = cropToGrid(tile, latMin, latMax, lonMin, lonMax);
        std::cout << "OK  void outside window ignored (" << g.rows() << "x"
                  << g.cols() << ")\n";
    } catch (const std::exception& e) {
        std::cout << "BUG void outside window threw: " << e.what() << '\n';
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