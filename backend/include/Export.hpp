#pragma once
#include "Grid.hpp"
#include "GeoUtils.hpp"   // TileIndex, DemTile
#include <string>
#include <vector>

// Ένα «επικίνδυνο» σημείο για τον χάρτη.
struct SlopePoint {
    double lat;        // γεωγραφικό πλάτος, μοίρες
    double lon;        // γεωγραφικό μήκος, μοίρες
    double elevation;  // ύψος, μέτρα
    double slope;      // κλίση, μοίρες [0, 90]
};

// Τα πιο απότομα σημεία ενός παραθύρου, σε φθίνουσα σειρά κλίσης.
// - heights, slopes: Grid ίδιου μεγέθους (ύψη και κλίσεις του παραθύρου)
// - tile, origin: το tile και η βορειοδυτική γωνία του παραθύρου μέσα σε αυτό,
//   ώστε το (i, j) του Grid να αντιστοιχεί στο (origin.r + i, origin.c + j) του tile
// - n: μέγιστο πλήθος σημείων που επιστρέφονται
// - minDistanceM: ελάχιστη απόσταση σε μέτρα ανάμεσα σε δύο σημεία που κρατιούνται
//   (για να μη μαζεύονται όλα στον ίδιο γκρεμό· 0 = χωρίς περιορισμό)
// Πετάει std::invalid_argument αν n <= 0, αν minDistanceM < 0 ή δεν είναι
// πεπερασμένο, ή αν heights και slopes έχουν διαφορετικό μέγεθος.
std::vector<SlopePoint> topSteepest(const Grid& heights, const Grid& slopes,
                                    const DemTile& tile, TileIndex origin,
                                    int n, double minDistanceM);

// Γράφει τα σημεία σε CSV με κεφαλίδα: lat,lon,elevation,slope,difficulty
// (η difficulty προκύπτει από το slope με classify). Το αρχείο αντικαθίσταται
// αν υπάρχει. Πετάει std::runtime_error αν δεν μπορεί να ανοίξει το αρχείο.
void writeSlopeCsv(const std::string& path, const std::vector<SlopePoint>& points);