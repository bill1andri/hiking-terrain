#pragma once
#include "Conditions.hpp"
#include <string>
#include <vector>

// Overall safety verdict ordered from safest (Go) to most hazardous (NoGo).
enum class Verdict {
    Go, Caution, NoGo
};

// Configurable safety thresholds for weather and daylight rules.
struct Thresholds {
    double gusts_caution_kmh = 40.0;      // Wind gusts threshold for Caution (km/h)
    double gusts_nogo_kmh = 70.0;         // Wind gusts threshold for NoGo (km/h)

    int precip_caution_pct = 40;          // Precipitation probability for Caution (%)
    int precip_nogo_pct = 80;             // Precipitation probability for NoGo (%)

    double cold_caution_c = 0.0;          // Wind-chill temperature for Caution (C)
    double cold_nogo_c = -10.0;           // Wind-chill temperature for NoGo (C)

    double heat_caution_c = 32.0;         // Target-elevation temperature for Caution (C)
    double heat_nogo_c = 38.0;            // Target-elevation temperature for NoGo (C)

    int thunderstorm_code_min = 95;       // Minimum WMO code indicating thunderstorm
    int thunderstorm_code_max = 99;       // Maximum WMO code indicating thunderstorm

    int daylight_buffer_min = 60;         // Required safety margin before sunset (minutes)
};

// Combined safety evaluation result containing the worst verdict and all triggered reasons.
struct Assessment {
    Verdict verdict = Verdict::Go;
    std::vector<std::string> reasons;
};

// Evaluates weather and daylight conditions at target_elev_m against safety thresholds.
Assessment evaluateSafety(const Conditions& cond, double target_elev_m, int daylight_left_min = 600, int duration_min = 0, const Thresholds& t = Thresholds{});

// Converts a Verdict enum value to its uppercase string representation ("GO", "CAUTION", "NO_GO").
std::string toString(Verdict v);