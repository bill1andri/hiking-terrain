#include "Cache.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

ForecastCache::ForecastCache(std::chrono::seconds ttl) : ttl_(ttl) {}

std::string ForecastCache::makeKey(double lat, double lon, std::optional<double> elevation_m) {
    double r_lat = std::round(lat * 100.0) / 100.0;
    double r_lon = std::round(lon * 100) / 100.0;
    if(r_lat == 0.0) r_lat = 0.0;
    if(r_lon == 0.0) r_lon = 0.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << r_lat << "," << r_lon << ":";
    if(elevation_m.has_value()) {
        oss << static_cast<int>(std::round(*elevation_m));
    } else {
        oss << "auto";
    }

    return oss.str();
}

std::optional<std::vector<HourlyForecast>> ForecastCache::get(double lat, double lon, std::optional<double> elevation_m) {
    std::string key = makeKey(lat, lon, elevation_m);
    auto it = store_.find(key);
    if(it == store_.end()) {
        return std::nullopt;
    }

    auto age = std::chrono::steady_clock::now() - it->second.inserted_at;
    if(age >= ttl_) {
        store_.erase(it);
        return std::nullopt;
    }

    return it->second.forecast;
} 

void ForecastCache::put(double lat, double lon, std::optional<double> elevation_m, const std::vector<HourlyForecast>& forecast) {
    std::string key = makeKey(lat, lon, elevation_m);
    store_[key] = Entry{forecast, std::chrono::steady_clock::now()};
}