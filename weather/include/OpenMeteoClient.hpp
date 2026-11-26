#pragma once
#include "Conditions.hpp"
#include <optional>
#include <string>
#include <vector>

struct HourlyForecast {
    std::string time;

    Conditions conditions;

    std::string sunset;

    int daylight_left_min = 0;
};

class OpenMeteoClient {
    private:
        std::string host_;
        int timeout_sec_;

    public:
        explicit OpenMeteoClient(const std::string& host = "https://api.open-meteo.com", int timeout_sec = 5);
        std::vector<HourlyForecast> fetchForecast(double lat, double lon, std::optional<double> elevation_m = std::nullopt, int forecast_days = 2) const;
        static std::vector<HourlyForecast> parseResponse(const std::string& json_body);
};