#include "Crop.hpp"       // δικό σου header πρώτο (φέρνει Grid.hpp + DemLoader.hpp)
#include "GeoUtils.hpp"   // latLonToIndex, indexToLatLon, cellSizeAt
#include <cstdint>        // std::int16_t
#include <stdexcept>      // invalid_argument, out_of_range, runtime_error
#include <string>         // std::to_string

// Τιμή που βάζει το SRTM όταν δεν υπάρχει μέτρηση (void).
constexpr std::int16_t kVoid = -32768;

Grid cropToGrid(const DemTile& tile, double latMin, double latMax,
                double lonMin, double lonMax) {
    // 1. Έλεγχος παραθύρου
    if (latMin >= latMax || lonMin >= lonMax) {
        throw std::invalid_argument(
            "cropToGrid: need latMin < latMax and lonMin < lonMax, got lat [" +
            std::to_string(latMin) + ", " + std::to_string(latMax) + "], lon [" +
            std::to_string(lonMin) + ", " + std::to_string(lonMax) + "]");
    }

    // 2. Γωνίες -> indices (πετάει out_of_range αν είναι έξω από το tile)
    TileIndex nw = latLonToIndex(tile, latMax, lonMin);  // πάνω-αριστερά
    TileIndex se = latLonToIndex(tile, latMin, lonMax);  // κάτω-δεξιά
    int rMin = nw.r;
    int cMin = nw.c;

    // 3. Διαστάσεις (σημεία, όχι διαστήματα: +1)
    int rows = se.r - nw.r + 1;
    int cols = se.c - nw.c + 1;
    if (rows < 2 || cols < 2) {
        throw std::invalid_argument(
            "cropToGrid: window too small: " + std::to_string(rows) + "x" +
            std::to_string(cols) + " points, need at least 2x2");
    }

    // 4. Μέγεθος κελιού στο κέντρο του παραθύρου
    double latMid = (latMin + latMax) / 2.0;
    CellSize cs = cellSizeAt(latMid, tile.size() - 1);

    // 5-6. Αντιγραφή υψών
    Grid heights(rows, cols, cs.dx, cs.dy);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::int16_t h = tile.at(rMin + i, cMin + j);
            if (h == kVoid) {
                LatLon p = indexToLatLon(tile, rMin + i, cMin + j);
                throw std::runtime_error(
                    "cropToGrid: no elevation data (void) at (" +
                    std::to_string(p.lat) + ", " + std::to_string(p.lon) + ")");
            }
            heights.set(i, j, h);
        }
    }

    // 7.
    return heights;
}