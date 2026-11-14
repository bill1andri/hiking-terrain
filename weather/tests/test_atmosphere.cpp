// Unit tests for atmospheric calculations (lapse rate and wind chill).
#include "Atmosphere.hpp"
#include <cmath>
#include <iostream>

int main() {
    int failures = 0; // Error counter
    
    // Test 1: Elevation gain of 1000 m (100 m -> 1100 m) drops temperature by 6.5 C.
    double t = temperatureAtElevation(20.0, 100.0, 1100.0);
    if(std::abs(t - 13.5) > 0.001) {
        std::cerr << "FAIL: temperatureAtElevation "
                  << "expected 13.5, got " << t << "\n";
        failures++;
    }

    // Test 2: Wind chill active (T = -10 C, wind = 30 km/h -> approx. -19.5 C).
    double w1 = windChill(-10.0, 30.0);
    if(std::abs(w1 + 19.5) > 0.1) {
        std::cerr << "FAIL: windChill "
                  << "expected -19.5, got " << w1 << "\n";
        failures++;
    }

    // Test 3: Wind chill inactive when temperature > 10 C (returns original temperature).
    double w2 = windChill(15.0, 30.0);
    if(std::abs(w2 - 15.0) > 0.001) {
        std::cerr << "FAIL: windChill "
                  << "expected 15.0, got " << w2 << "\n";
        failures++;
    }

    // Test 4: Wind chill inactive when wind speed <= 4.8 km/h (returns original temperature).
    double w3 = windChill(0.0, 3.0);
    if(std::abs(w3 - 0.0) > 0.001) {
        std::cerr << "FAIL: windChill "
                  << "expected exactly 0.0, got " << w3 << "\n";
        failures++;
    }

    if(failures == 0) {
        std::cout << "All atmosphere tests have passed!\n";
        return 0;
    }

    return 1;
}