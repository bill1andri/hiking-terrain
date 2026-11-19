#include "DemLoader.hpp"
#include "Crop.hpp"
#include "Gradient.hpp"
#include "Difficulty.hpp"
#include "GeoUtils.hpp"
#include <iostream>
#include <iomanip>
#include <exception>
#include "Export.hpp"
#include <filesystem>

int main(int argc, char* argv[]) {
    try {
        const char* path = (argc > 1) ? argv[1] : "data/dem/N37E023.hgt";
        DemTile tile = loadHgt(path, 37.0, 23.0);

        // Παράθυρο γύρω από την κορυφή του Υμηττού
        const double latMin = 37.9233;
        const double latMax = 37.9567;
        const double lonMin = 23.8033;
        const double lonMax = 23.8367;

        Grid heights = cropToGrid(tile, latMin, latMax, lonMin, lonMax);
        std::cout << "Grid: " << heights.rows() << " x " << heights.cols() << " points\n";
        std::cout << "dx = " << heights.dx() << " m, dy = " << heights.dy() << " m\n";

        // --- Μέρος 2: κλίσεις και μέγιστη κλίση ---

        // 1. Κλίσεις σε μοίρες για κάθε κελί
        Grid slopes = slopeGrid(heights);

        // 2. Argmax: μέγιστη κλίση και πού βρίσκεται (+ άθροισμα για μέσο όρο)
        double maxSlope = -1.0;
        int maxI = 0;
        int maxJ = 0;
        double sum = 0.0;

        for (int i = 0; i < slopes.rows(); i++) {
            for (int j = 0; j < slopes.cols(); j++) {
                double s = slopes.at(i, j);
                sum += s;
                if (s > maxSlope) {
                    maxSlope = s;
                    maxI = i;
                    maxJ = j;
                }
            }
        }
        double mean = sum / (slopes.rows() * slopes.cols());

        // 3. Θέση μέσα στο Grid -> θέση στο tile -> lat/lon
        TileIndex nw = latLonToIndex(tile, latMax, lonMin);   // πάνω-αριστερά
        LatLon where = indexToLatLon(tile, nw.r + maxI, nw.c + maxJ);

        // 4. Εκτύπωση
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Mean slope: " << mean << " deg ("
                  << toString(classify(mean)) << ")\n";
        std::cout << "Steepest point: " << maxSlope << " deg ("
                  << toString(classify(maxSlope)) << ")\n";
        std::cout << std::setprecision(5);
        std::cout << "  at lat " << where.lat << ", lon " << where.lon
                  << ", elevation " << std::setprecision(0)
                  << heights.at(maxI, maxJ) << " m\n";
        std::filesystem::create_directories("output");
        std::vector<SlopePoint> top = topSteepest(heights, slopes, tile, nw, 20, 100.0);
        writeSlopeCsv("output/steepest.csv", top);
        std::cout << "Wrote " << top.size() << " points to output/steepest.csv\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

