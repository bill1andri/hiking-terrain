#pragma once
#include "Grid.hpp"
#include "DemLoader.hpp"

// Αντιγράφει το παράθυρο [latMin, latMax] x [lonMin, lonMax] ενός DemTile σε Grid.
// - Γραμμή 0 του Grid = βορράς (latMax), στήλη 0 = δύση (lonMin)
// - dx, dy του Grid: πραγματικά μέτρα, υπολογισμένα στο κεντρικό πλάτος
//   του παραθύρου (dx μικραίνει όσο ανεβαίνουμε βόρεια, dy σταθερό)
// Πετάει std::invalid_argument αν latMin >= latMax, αν lonMin >= lonMax ή αν
// το παράθυρο έχει λιγότερα από 2x2 σημεία, std::out_of_range αν το παράθυρο
// βγαίνει έξω από το tile, και std::runtime_error αν μέσα στο παράθυρο
// υπάρχει void (-32768).
Grid cropToGrid(const DemTile& tile, double latMin, double latMax, double lonMin, double lonMax);
