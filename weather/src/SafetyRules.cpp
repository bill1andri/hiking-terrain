#include "SafetyRules.hpp"
#include "Atmosphere.hpp"
#include <cmath>
#include <stdexcept>

// Maps each Verdict enum value to its API string representation.
std::string toString(Verdict v) {
    if(v == Verdict::Go) return "GO";
    if(v == Verdict::Caution) return "CAUTION";
    return "NO_GO";
}

// Validates inputs, adjusts temperature/wind-chill for elevation, and applies all 6 safety rules.
Assessment evaluateSafety(const Conditions& cond, double target_elev_m, int daylight_left_min, int duration_min, const Thresholds& t) {
    // Reject non-finite floating-point inputs (NaN / Inf).
    if(!std::isfinite(cond.temperature_c) || !std::isfinite(cond.wind_kmh) || !std::isfinite(cond.gusts_kmh)  
       || !std::isfinite(cond.elevation_m) || !std::isfinite(target_elev_m)) {
        throw std::invalid_argument("Argument is not a real number!");
    }

    // Fail-safe validation against physically impossible input values.
    if(cond.temperature_c < -90.0 || cond.temperature_c > 60.0) {
        throw std::invalid_argument("Temperature out of normal range!");
    }

    if(cond.wind_kmh < 0.0 || cond.gusts_kmh < 0.0) {
        throw std::invalid_argument("Wind/Gusts out of normal range!");
    }

    if(cond.precip_probability_pct < 0 || cond.precip_probability_pct > 100) {
        throw std::invalid_argument("Precipitation probability of rain out of range!");
    }

    if(cond.weather_code < 0 || cond.weather_code > 99) {
        throw std::invalid_argument("Weather code out of range!");
    }

    if(cond.elevation_m < -500 || cond.elevation_m > 9000 || target_elev_m < -500 || target_elev_m > 9000|| duration_min < 0) {
        throw std::invalid_argument("Elevation/Duration of hiking session out of range!");
    }

    // Compute temperature and wind chill at the target hiking elevation.
    double temp_at_elev = temperatureAtElevation(cond.temperature_c, cond.elevation_m, target_elev_m);
    double feels_like = windChill(temp_at_elev, cond.wind_kmh);

    Assessment result;

    // Rule 1: Gusts safety check (NoGo > Caution > Go).
    if(cond.gusts_kmh > t.gusts_nogo_kmh) {
        result.verdict = Verdict::NoGo;
        result.reasons.push_back("Gusts above 70 km/h!");
    }
    else if(cond.gusts_kmh > t.gusts_caution_kmh) {
        if(result.verdict == Verdict::Go) {
            result.verdict = Verdict::Caution;
        }
        result.reasons.push_back("Gusts above 40 km/h!");
    }

    // Rule 2: Rain precipitation probability safety check.
    if(cond.precip_probability_pct > t.precip_nogo_pct) {
        result.verdict = Verdict::NoGo;
        result.reasons.push_back("Precipitation probability above 80%!");
    }
    else if(cond.precip_probability_pct > t.precip_caution_pct) {
        if(result.verdict == Verdict::Go) {
            result.verdict = Verdict::Caution;
        }
        result.reasons.push_back("Precipitation probability above 40%!");
    }

    // Rule 3: Cold (feels-like / wind chill) safety check.
    if(feels_like < t.cold_nogo_c) {
        result.verdict = Verdict::NoGo;
        result.reasons.push_back("Feels like lower than -10 Celsius degrees!");
    }
    else if(feels_like < t.cold_caution_c) {
        if(result.verdict == Verdict::Go) {
            result.verdict = Verdict::Caution;
        }
        result.reasons.push_back("Feels like lower than 0 Celsius degrees!");
    }

    // Rule 4: Heat (temperature at target elevation) safety check.
    if(temp_at_elev > t.heat_nogo_c) {
        result.verdict = Verdict::NoGo;
        result.reasons.push_back("Temperature at target elevation higher than 38 Celsius degrees!");
    }
    else if(temp_at_elev > t.heat_caution_c) {
        if(result.verdict == Verdict::Go) {
            result.verdict = Verdict::Caution;
        }
        result.reasons.push_back("Temperature at target elevation higher than 32 Celsius degrees!");
    }

    // Rule 5: Thunderstorm (WMO weather code 95-99) safety check.
    if(cond.weather_code >= t.thunderstorm_code_min && cond.weather_code <= t.thunderstorm_code_max) {
        result.verdict = Verdict::NoGo;
        result.reasons.push_back("High risk because of rain storm!");
    }

    // Rule 6: Remaining daylight vs. hike duration safety check.
    if(daylight_left_min < duration_min) {
        result.verdict = Verdict::NoGo;
        result.reasons.push_back("Not enough daylight to complete the hike!");
    }
    else if(daylight_left_min < duration_min + t.daylight_buffer_min) {
        if(result.verdict == Verdict::Go) {
            result.verdict = Verdict::Caution;
        }
        result.reasons.push_back("Less than 60 minutes of daylight buffer before sunset!");
    }

    return result;
}