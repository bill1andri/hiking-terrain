// Μικρό πρόγραμμα για να "κοιτάξουμε" ένα tile .hgt.
// Χρήση: ./build/dem_probe [path]   (προεπιλογή: data/dem/N37E023.hgt)
#include "DemLoader.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

static constexpr std::int16_t kVoid = -32768;  // "κενό": δεν υπάρχει μέτρηση

int main(int argc, char* argv[]) {
    const std::string path = (argc >= 2) ? argv[1] : "data/dem/N37E023.hgt";

    try {
        DemTile t = loadHgt(path, 37.0, 23.0);
        const int n = t.size();
        std::cout << "Loaded " << path << ": " << n << " x " << n << " points\n\n";

        // ---------- 1. Όλο το tile: ελάχιστο, μέγιστο, κενά ----------
        int minH = 100000, maxH = -100000;
        long voids = 0;
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                const std::int16_t h = t.at(r, c);
                if (h == kVoid) {
                    voids++;
                    continue;  // τα κενά δεν μετράνε στο min/max
                }
                if (h < minH) minH = h;
                if (h > maxH) maxH = h;
            }
        }
        std::cout << "Whole tile:\n"
                  << "  lowest  " << minH << " m\n"
                  << "  highest " << maxH << " m\n"
                  << "  voids   " << voids << " points\n\n";

        // ---------- 2. Παράθυρο γύρω από τον Υμηττό ----------
        // κορυφή ~ 37.94 N, 23.82 E  ->  r ~ (38 - 37.94) * 3600 = 216,  c ~ 0.82 * 3600 = 2952
        const int r0 = 156, r1 = 276, c0 = 2892, c1 = 3012;
        if (n == 3601) {
            int best = kVoid, bestR = -1, bestC = -1;
            for (int r = r0; r <= r1; r++) {
                for (int c = c0; c <= c1; c++) {
                    const std::int16_t h = t.at(r, c);
                    if (h != kVoid && h > best) {
                        best = h;
                        bestR = r;
                        bestC = c;
                    }
                }
            }
            // από (r, c) πίσω σε γεωγραφικές συντεταγμένες
            const double lat = t.latSouth() + 1.0 - bestR / 3600.0;
            const double lon = t.lonWest() + bestC / 3600.0;
            std::cout << "Hymettus window (rows " << r0 << "-" << r1
                      << ", cols " << c0 << "-" << c1 << "):\n"
                      << "  highest point " << best << " m at (r=" << bestR
                      << ", c=" << bestC << ")\n"
                      << "  approx. lat " << lat << ", lon " << lon << '\n';
        } else {
            std::cout << "(Hymettus window skipped: tile is not 3601 x 3601)\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}