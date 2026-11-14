// Unit tests for hiking safety rules, boundary thresholds, and fail-safe input validation.
#include "SafetyRules.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    int failures = 0; // Error counter

    // Test 1: Ideal weather conditions -> GO with no warning reasons.
    Conditions c{18.0, 10.0, 20.0, 10, 0, 200.0};
    Assessment r = evaluateSafety(c, 500.0);
    if(r.verdict != Verdict::Go || !r.reasons.empty()) {
        std::cerr << "FAIL: expected GO, got " << toString(r.verdict) << "\n";
        failures++;
    }
    
    // Test 2: Exact boundary testing for wind gusts (40.0/40.1 and 70.0/70.1 km/h).
    c.gusts_kmh = 40.0; // Exactly 40 -> GO
    if(evaluateSafety(c, 500.0).verdict != Verdict::Go) {
        std::cerr << "Fail: gusts 40.0km/h should be GO\n";
        failures++;
    }

    c.gusts_kmh = 40.1; // Over 40 -> CAUTION
    if(evaluateSafety(c, 500.0).verdict != Verdict::Caution) {
        std::cerr << "Fail: gusts 40.1km/h should be CAUTION\n";
        failures++;
    }

    c.gusts_kmh = 70.0; // Exactly 70 -> CAUTION
    if(evaluateSafety(c, 500.0).verdict != Verdict::Caution) {
        std::cerr << "Fail: gusts 70.0km/h should be CAUTION\n";
        failures++;
    }

    c.gusts_kmh = 70.1; // Over 70 -> NO_GO
    if(evaluateSafety(c, 500.0).verdict != Verdict::NoGo) {
        std::cerr << "Fail: gusts 70.1km/h should be NO_GO\n";
        failures++;
    }

    // Test 3: Combination of NO_GO (gusts) + CAUTION (rain) -> worst verdict (NO_GO) and both reasons kept.
    c.precip_probability_pct = 50; // -> CAUTION
    Assessment combo = evaluateSafety(c, 500.0);
    if(combo.verdict != Verdict::NoGo || combo.reasons.size() != 2) {
        std::cerr << "FAIL: combo should be NO_GO with 2 reasons\n";
        failures++;
    }

    // Test 4: Remaining daylight vs. hike duration checks.
    Conditions good{18.0, 10.0, 20.0, 10, 0, 200.0};
    if(evaluateSafety(good, 500.0, 150, 120).verdict != Verdict::Caution) {
        std::cerr << "FAIL: 30min daylight buffer should be CAUTION\n";
        failures++;
    }

    if(evaluateSafety(good, 500.0, 100, 120).verdict != Verdict::NoGo) {
        std::cerr << "FAIL: insufficient daylight should be NO_GO\n";
        failures++;
    }

    // Test 5: Fail-safe rejection of physically impossible inputs (400 C throws std::invalid_argument).
    Conditions bad = good;
    bad.temperature_c = 400.0;
    bool threw = false;

    try {
        evaluateSafety(bad, 500.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    if(!threw) {
        std::cerr << "FAIL: expected invalid_arguments for 400 C\n";
        failures++;
    }

    if(failures == 0) {
        return 0;
    }

    return 1;
}