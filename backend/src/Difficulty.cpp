#include "Difficulty.hpp"
#include <cmath>
#include <stdexcept>

Difficulty classify(double degrees, const DifficultyThresholds& t) {
    // 1. Έλεγχος εισόδου: nan, < 0 ή > 90 -> invalid_argument
    if (std::isnan(degrees)) {
        throw std::invalid_argument("classify: angle is NaN");
    }
    if (degrees < 0.0 || degrees > 90.0) {
        throw std::invalid_argument("classify: angle out of range [0, 90]: " +
                                    std::to_string(degrees));
    }

    // 2. Από το πιο απότομο προς το πιο εύκολο
    if (degrees >= t.verySteep) return Difficulty::VerySteep;
    if (degrees >= t.steep)     return Difficulty::Steep;
    if (degrees >= t.moderate)  return Difficulty::Moderate;
    return Difficulty::Easy;
}

std::string toString(Difficulty d) {
    switch (d) {
        case Difficulty::Easy:      return "easy";
        case Difficulty::Moderate:  return "moderate";
        case Difficulty::Steep:     return "steep";
        case Difficulty::VerySteep: return "very_steep";
    }
    // Δεν φτάνει ποτέ εδώ, αλλά ο compiler θέλει κάτι μετά το switch.
    throw std::logic_error("toString: invalid Difficulty value");
}
