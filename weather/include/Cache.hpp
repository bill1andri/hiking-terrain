#pragma once
#include "OpenMeteoClient.hpp"
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class ForecastCache {
    private:
        struct Entry {
            std::vector<HourlyForecast> forecast;
            std::chrono::steady_clock::time_point inserted_at;
        };

        std::chrono::seconds ttl_;
        std::unordered_map<std::string, Entry> store_;

    public:
        //Default TTL: 30 minutes (1800 seconds)
        explicit ForecastCache(std::chrono::seconds ttl = std::chrono::minutes(30));

        //Returns cached forecast if present and not expired, otherwise std::nullopt
        std::optional<std::vector<HourlyForecast>> get(double lat, double lon, std::optional<double> elevation_m = std::nullopt);

        //Stores forecast in cache with current timestamp
        void put(double lat, double lon, std::optional<double> elevation_m, const std::vector<HourlyForecast>& forecast);

        //Build rounded cache key(2 decimal places for lat/lon)
        static std::string makeKey(double lat, double lon, std::optional<double> elevation_m = std::nullopt);
};