#pragma once
#include <string>

enum class Difficulty {Easy, Moderate, Steep, VerySteep};

// Τα όρια σε μοίρες. Από κάθε τιμή και πάνω ξεκινά η αντίστοιχη κατηγορία.
// Οι τιμές μετά το = είναι οι προεπιλογές.
struct DifficultyThresholds {
    double moderate = 10.0;
    double steep = 20.0;
    double verySteep = 30.0;
};

// Γωνία σε μοίρες -> κατηγορία. Πετάει std::invalid_argument για
// γωνία εκτός [0, 90] ή nan.
Difficulty classify(double degrees,
                    const DifficultyThresholds& t = DifficultyThresholds{});

// Κατηγορία -> κείμενο ("easy", "moderate", "steep", "very_steep").
std::string toString(Difficulty d);