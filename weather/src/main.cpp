// Phase 2 CLI: fetches live Open-Meteo forecast for Mt. Hymettus (1026 m),
// compares lapse-rate adjustment, and prints hourly safety verdicts for tomorrow.
#include "Atmosphere.hpp"
#include "Cache.hpp"
#include "OpenMeteoClient.hpp"
#include "SafetyRules.hpp"
#include <exception>
#include <iomanip>
#include <iostream>

// Helper that checks ForecastCache before calling OpenMeteoClient over HTTPS.
static std::vector<HourlyForecast> getForecastCached(
    OpenMeteoClient& client,
    ForecastCache& cache,
    double lat,
    double lon,
    std::optional<double> elevation_m) {
    if (auto cached = cache.get(lat, lon, elevation_m)) {
        return *cached;
    }
    auto fresh = client.fetchForecast(lat, lon, elevation_m, 2);
    cache.put(lat, lon, elevation_m, fresh);
    return fresh;
}

int main() {
    constexpr double HYMETTUS_LAT = 37.94;
    constexpr double HYMETTUS_LON = 23.82;
    constexpr double HYMETTUS_ELEV_M = 1026.0;
    constexpr int HIKE_DURATION_MIN = 180; // 3-hour hike

    std::cout << "Note: This verdict is an advisory aid, not a guarantee of safety.\n"
              << "Weather data by Open-Meteo.com (CC BY 4.0)\n\n";

    try {
        OpenMeteoClient client;
        ForecastCache cache;

        // 1. Fetch forecast at summit elevation (1026 m) and default model elevation
        auto summit_hours = getForecastCached(client, cache, HYMETTUS_LAT, HYMETTUS_LON, HYMETTUS_ELEV_M);
        auto raw_hours = getForecastCached(client, cache, HYMETTUS_LAT, HYMETTUS_LON, std::nullopt);

        if (summit_hours.size() < 48 || raw_hours.size() < 48) {
            std::cerr << "Error: incomplete 2-day forecast received.\n";
            return 1;
        }

        // 2. Compare Open-Meteo elevation correction vs. Phase 1 lapse-rate formula (at 12:00 tomorrow)
        const auto& om_noon = summit_hours[36];
        const auto& raw_noon = raw_hours[36];
        double phase1_temp = temperatureAtElevation(
            raw_noon.conditions.temperature_c, raw_noon.conditions.elevation_m, HYMETTUS_ELEV_M);

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "=== Elevation Temperature Comparison (" << om_noon.time << ") ===\n"
                  << "  Model grid elevation (" << raw_noon.conditions.elevation_m << " m): "
                  << raw_noon.conditions.temperature_c << " C\n"
                  << "  Open-Meteo at 1026 m:           " << om_noon.conditions.temperature_c << " C\n"
                  << "  Phase 1 lapse-rate at 1026 m:   " << phase1_temp << " C\n\n";

        // 3. Print hourly verdicts for tomorrow (hours 24..47)
        std::cout << "=== Mt. Hymettus Summit (1026 m) - Tomorrow's Hourly Verdicts (3h hike) ===\n";
        for (std::size_t i = 24; i < 48; i++) {
            const auto& h = summit_hours[i];
            double feels_like = windChill(h.conditions.temperature_c, h.conditions.wind_kmh);
            Assessment res = evaluateSafety(
                h.conditions, HYMETTUS_ELEV_M, h.daylight_left_min, HIKE_DURATION_MIN);

            std::cout << h.time
                      << " | Temp: " << std::setw(5) << h.conditions.temperature_c << " C"
                      << " | Feels: " << std::setw(5) << feels_like << " C"
                      << " | Gusts: " << std::setw(4) << h.conditions.gusts_kmh << " km/h"
                      << " | " << std::setw(7) << toString(res.verdict);

            if (!res.reasons.empty()) {
                std::cout << " (" << res.reasons[0];
                for (std::size_t r = 1; r < res.reasons.size(); r++) {
                    std::cout << "; " << res.reasons[r];
                }
                std::cout << ")";
            }
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching forecast: " << e.what() << "\n";
        return 1;
    }

    return 0;
}