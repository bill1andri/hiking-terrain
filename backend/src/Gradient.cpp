#include "Gradient.hpp"
#include "Grid.hpp"
#include <cmath>
#include "GeoUtils.hpp"
// ∂h/∂x: κινούμαστε στις ΣΤΗΛΕΣ (c)
static double partialX(const Grid& g, int r, int c) {
    if (g.cols() == 1) {
        return 0.0;  // δεν υπάρχει γείτονας κατά x
    }
    if (c == 0) {  // αριστερή άκρη: μονόπλευρη προς τα δεξιά
        return (g.at(r, 1) - g.at(r, 0)) / g.dx();
    }
    if (c == g.cols() - 1) {  // δεξιά άκρη: μονόπλευρη προς τα αριστερά
        return (g.at(r, c) - g.at(r, c - 1)) / g.dx();
    }
    // εσωτερικό: κεντρική διαφορά
    return (g.at(r, c + 1) - g.at(r, c - 1)) / (2.0 * g.dx());
}

// ∂h/∂y: κινούμαστε στις ΓΡΑΜΜΕΣ (r)
static double partialY(const Grid& g, int r, int c) {
    if (g.rows() == 1) {
        return 0.0;  // δεν υπάρχει γείτονας κατά y
    }
    if (r == 0) {  // πάνω άκρη: μονόπλευρη προς τα κάτω
        return (g.at(1, c) - g.at(0, c)) / g.dy();
    }
    if (r == g.rows() - 1) {  // κάτω άκρη: μονόπλευρη προς τα πάνω
        return (g.at(r, c) - g.at(r - 1, c)) / g.dy();
    }
    // εσωτερικό: κεντρική διαφορά
    return (g.at(r + 1, c) - g.at(r - 1, c)) / (2.0 * g.dy());
}

Gradient gradientAt(const Grid& g, int r, int c) { 
    return Gradient {partialX(g, r, c), partialY(g, r, c)};
}


double slopeDegrees(const Gradient& grad) {
    // 1. μέτρο του διανύσματος = πόσο ανεβαίνεις ανά μέτρο (tan θ)
    double magnitude = std::sqrt(grad.dx * grad.dx + grad.dy * grad.dy);

    // 2. από tan θ στη γωνία, σε ακτίνια
    double radians = std::atan(magnitude);

    // 3. από ακτίνια σε μοίρες
    return radToDeg(radians);
}

Grid slopeGrid(const Grid& heights) { 
    Grid slopes(heights.rows(), heights.cols(), heights.dx(), heights.dy());
    for (int r = 0; r < heights.rows(); r++) {
        for (int c = 0; c < heights.cols(); c++) {
            slopes.set(r, c, slopeDegrees(gradientAt(heights, r, c)));
        }
    }
    return slopes;
}