#pragma once
#include "DemLoader.hpp"

constexpr double kPi = 3.14159265358979323846;
constexpr double kEarthRadiusM = 6371000.0;  // μέση ακτίνα της Γης, σε μέτρα

// Μέγεθος ενός κελιού του πλέγματος, σε μέτρα.
struct CellSize {
    double dx;  // east-west, metres
    double dy;  // north-south, metres
};

// Θέση σημείου μέσα σε ένα tile (γραμμή 0 = βορράς, στήλη 0 = δύση).
struct TileIndex {
    int r;
    int c;
};

// Γεωγραφικές συντεταγμένες, σε μοίρες.
struct LatLon {
    double lat;  // γεωγραφικό πλάτος (latitude)
    double lon;  // γεωγραφικό μήκος (longitude)
};

// Μετατροπές γωνιών.
double degToRad(double degrees);
double radToDeg(double radians);

// Μέτρα ανά μοίρα.
double metersPerDegreeLat();
double metersPerDegreeLon(double latDegrees);

// Μέγεθος κελιού σε δοσμένο πλάτος. Πετάει std::invalid_argument
// για πλάτος εκτός [-90, 90] / μη πεπερασμένο ή intervalsPerDegree <= 0.
CellSize cellSizeAt(double latDegrees, int intervalsPerDegree);

// (lat, lon) -> (r, c) του πλησιέστερου σημείου. Πετάει std::invalid_argument
// για άκυρες συντεταγμένες και std::out_of_range αν είναι έξω από το tile.
TileIndex latLonToIndex(const DemTile& tile, double latDegrees, double lonDegrees);

// (r, c) -> (lat, lon). Πετάει std::out_of_range αν το (r, c) είναι έξω από το tile.
LatLon indexToLatLon(const DemTile& tile, int r, int c);
