#include "Export.hpp"       // δικό σου header πρώτο (φέρνει Grid, GeoUtils, string, vector)
#include "Difficulty.hpp"   // classify, toString (για τη στήλη difficulty)
#include <algorithm>        // std::sort
#include <cmath>            // std::sqrt, std::isfinite
#include <fstream>          // std::ofstream
#include <iomanip>          // std::fixed, std::setprecision
#include <stdexcept>        // invalid_argument, runtime_error

// Βοηθητικό: ένα σημείο του Grid με την κλίση του (μόνο σε αυτό το αρχείο).
struct Candidate {
    int i;
    int j;
    double slope;
};

std::vector<SlopePoint> topSteepest(const Grid& heights, const Grid& slopes,
                                    const DemTile& tile, TileIndex origin,
                                    int n, double minDistanceM) {
    // --- Φάση 1: έλεγχοι ---
    if (n <= 0) {
        throw std::invalid_argument(
            "topSteepest: n must be positive, got " + std::to_string(n));
    }
    if (!std::isfinite(minDistanceM) || minDistanceM < 0.0) {
        throw std::invalid_argument(
            "topSteepest: minDistanceM must be a finite number >= 0, got " +
            std::to_string(minDistanceM));
    }
    if (heights.rows() != slopes.rows() || heights.cols() != slopes.cols()) {
        throw std::invalid_argument(
            "topSteepest: heights is " + std::to_string(heights.rows()) + "x" +
            std::to_string(heights.cols()) + " but slopes is " +
            std::to_string(slopes.rows()) + "x" + std::to_string(slopes.cols()));
    }

    // --- Φάση 2: συλλογή όλων των σημείων ---
    std::vector<Candidate> all;
    all.reserve(slopes.rows() * slopes.cols());
    for (int i = 0; i < slopes.rows(); i++) {
        for (int j = 0; j < slopes.cols(); j++) {
            all.push_back(Candidate{i, j, slopes.at(i, j)});
        }
    }

    // --- Φάση 3: ταξινόμηση, πιο απότομα πρώτα ---
    std::sort(all.begin(), all.end(),
              [](const Candidate& a, const Candidate& b) {
                  return a.slope > b.slope;
              });

    // --- Φάση 4: επιλογή με ελάχιστη απόσταση (greedy) ---
    std::vector<Candidate> chosen;
    for (const Candidate& cand : all) {
        if (static_cast<int>(chosen.size()) >= n) {
            break;                                       // μαζέψαμε n
        }
        bool tooClose = false;
        for (const Candidate& k : chosen) {
            double di = (cand.i - k.i) * heights.dy();   // γραμμές = βορράς-νότος
            double dj = (cand.j - k.j) * heights.dx();   // στήλες  = ανατολή-δύση
            double dist = std::sqrt(di * di + dj * dj);
            if (dist < minDistanceM) {
                tooClose = true;
                break;                                   // ένας κοντινός αρκεί
            }
        }
        if (!tooClose) {
            chosen.push_back(cand);
        }
    }

    // --- Φάση 5: Candidate (θέση στο Grid) -> SlopePoint (lat/lon, ύψος) ---
    std::vector<SlopePoint> result;
    result.reserve(chosen.size());
    for (const Candidate& c : chosen) {
        LatLon p = indexToLatLon(tile, origin.r + c.i, origin.c + c.j);
        result.push_back(SlopePoint{p.lat, p.lon, heights.at(c.i, c.j), c.slope});
    }
    return result;
}

void writeSlopeCsv(const std::string& path, const std::vector<SlopePoint>& points) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("writeSlopeCsv: cannot open file for writing: " + path);
    }

    out << "lat,lon,elevation,slope,difficulty\n";
    out << std::fixed;
    for (const SlopePoint& pt : points) {
        out << std::setprecision(5) << pt.lat << ','
            << pt.lon << ','
            << std::setprecision(0) << pt.elevation << ','
            << std::setprecision(2) << pt.slope << ','
            << toString(classify(pt.slope)) << '\n';
    }

    if (!out) {
        throw std::runtime_error("writeSlopeCsv: error while writing: " + path);
    }
}