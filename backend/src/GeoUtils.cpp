#include "GeoUtils.hpp"   // δικό μας header πρώτο (φέρνει και το DemLoader.hpp)
#include <cmath>
#include <stdexcept>
#include <string>

// Βοηθητική: ορατή μόνο μέσα σε αυτό το αρχείο
static void validateLatLon(double lat, double lon) {
    if (!std::isfinite(lat) || !std::isfinite(lon)) {
        throw std::invalid_argument(
            "validateLatLon: latitude and longitude must be finite numbers");
    }
    if (lat < -90.0 || lat > 90.0) {
        throw std::invalid_argument(
            "validateLatLon: latitude must be in [-90, 90], got " +
            std::to_string(lat));
    }
    if (lon < -180.0 || lon > 180.0) {
        throw std::invalid_argument(
            "validateLatLon: longitude must be in [-180, 180], got " +
            std::to_string(lon));
    }
}

double degToRad(double degrees) {
    return degrees * kPi / 180.0;
}

double radToDeg(double radians) {
    return radians * 180.0 / kPi;
}

double metersPerDegreeLat() {
    return kEarthRadiusM * degToRad(1.0);          // s = R·θ, θ = 1° σε rad
}

double metersPerDegreeLon(double latDegrees) {
    validateLatLon(latDegrees, 0.0);               // 0.0 = «ψεύτικο» έγκυρο lon
    return metersPerDegreeLat() * std::cos(degToRad(latDegrees));
}

CellSize cellSizeAt(double latDegrees, int intervalsPerDegree) {
    if (intervalsPerDegree <= 0) {
        throw std::invalid_argument(
            "cellSizeAt: intervalsPerDegree must be positive, got " +
            std::to_string(intervalsPerDegree));
    }
    double dx = metersPerDegreeLon(latDegrees) / intervalsPerDegree;  // ελέγχει το lat
    double dy = metersPerDegreeLat() / intervalsPerDegree;
    return CellSize{dx, dy};
}


TileIndex latLonToIndex(const DemTile& tile, double latDegrees, double lonDegrees) {
    validateLatLon(latDegrees, lonDegrees);

    int n = tile.size();
    if (n < 2) {
        throw std::invalid_argument(
            "latLonToIndex: tile must have at least 2 points per side, got " +
            std::to_string(n));
    }

    double south = tile.latSouth();
    double north = south + 1.0;
    double west  = tile.lonWest();
    double east  = west + 1.0;

    if (latDegrees < south || latDegrees > north ||
        lonDegrees < west  || lonDegrees > east) {
        throw std::out_of_range(
            "latLonToIndex: point (" + std::to_string(latDegrees) + ", " +
            std::to_string(lonDegrees) + ") is outside tile");
    }

    int r = static_cast<int>(std::round((north - latDegrees) * (n - 1)));
    int c = static_cast<int>(std::round((lonDegrees - west) * (n - 1)));
    return TileIndex{r, c};
}

LatLon indexToLatLon(const DemTile& tile, int r, int c) {
    int n = tile.size();
    if (n < 2) {
        throw std::invalid_argument(
            "indexToLatLon: tile must have at least 2 points per side, got " +
            std::to_string(n));
    }
    if (r < 0 || r > n - 1 || c < 0 || c > n - 1) {
        throw std::out_of_range(
            "indexToLatLon: index (" + std::to_string(r) + ", " +
            std::to_string(c) + ") is outside tile of size " + std::to_string(n));
    }

    double north = tile.latSouth() + 1.0;
    double west  = tile.lonWest();
    double step  = 1.0 / (n - 1);          // μοίρες ανά κελί (double!)

    double lat = north - r * step;
    double lon = west  + c * step;
    return LatLon{lat, lon};
}