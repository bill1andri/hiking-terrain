#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "OpenMeteoClient.hpp"
#include "httplib.h"
#include "json.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

OpenMeteoClient::OpenMeteoClient(const std::string& host, int timeout_sec)
: host_(host), timeout_sec_(timeout_sec) {
    if(timeout_sec <= 0) {
        throw std::invalid_argument("Timeout must be positive");
    }
}

static int computeDaylightLeftMin(const std::string& time_iso, const std::string& sunset_iso) {
    if(time_iso.size() < 16 || sunset_iso.size() < 16) {
        return 0;
    }

    int now_min = std::stoi(time_iso.substr(11, 2)) * 60 + std::stoi(time_iso.substr(14, 2));
    int sunset_min = std::stoi(sunset_iso.substr(11, 2)) * 60 + std::stoi(sunset_iso.substr(14, 2));

    return (sunset_min > now_min) ? (sunset_min - now_min) : 0;
}

std::vector<HourlyForecast> OpenMeteoClient::parseResponse(const std::string& json_body) {
    //Security: reject excessively large responses(> 1 MB)
    constexpr std::size_t MAX_RESPONSE_BYTES = 1024 * 1024;
    if(json_body.size() > MAX_RESPONSE_BYTES) {
        throw std::runtime_error("Open-Meteo response exceeds size limit");
    }

    nlohmann::json j = nlohmann::json::parse(json_body);

    if(!j.contains("elevation") || !j.contains("hourly") || !j.contains("daily")) {
        throw std::runtime_error("Missing required top-level fields in Open-Meteo JSON");
    }

    double elev_m = j["elevation"].get<double>();
    const auto& hourly = j["hourly"];
    const auto& daily = j["daily"];

    //Map each date ("YYYY-MM-DD") -> its sunset timestamp ("YYYY-MM-DDTHH:MM")
    std::unordered_map<std::string, std::string> sunset_by_date;
    if(daily.contains("time") && daily.contains("sunset") && daily["time"].is_array() && daily["sunset"].is_array() && daily["time"].size() == daily["sunset"].size()) {
        for(std::size_t d = 0; d < daily["time"].size(); d++) {
            sunset_by_date[daily["time"][d].get<std::string>()] = daily["sunset"][d].get<std::string>();
        }
    }

    //Check all required hourly arrays exists
    const char* required_fields[] = {
        "time", "temperature_2m", "precipitation_probability", "weather_code", "wind_speed_10m", "wind_gusts_10m"
    };
    for (const char* field : required_fields) {
        if (!hourly.contains(field) || !hourly[field].is_array()) {
            throw std::runtime_error(std::string("Missing hourly field: ") + field);
        }
    }

    const auto& times = hourly["time"];
    const auto& temps = hourly["temperature_2m"];
    const auto& precips = hourly["precipitation_probability"];
    const auto& codes = hourly["weather_code"];
    const auto& winds = hourly["wind_speed_10m"];
    const auto& gusts = hourly["wind_gusts_10m"];

    std::size_t count = times.size();
    if(temps.size() != count || precips.size() != count || codes.size() != count || winds.size() != count || gusts.size() != count) {
        throw std::runtime_error("Mismatched array lengths in hourly forecast");
    }

    std::vector<HourlyForecast> result;
    result.reserve(count);

    for(std::size_t i = 0; i < count; i++) {
        //Security reject null values inside JSON arrays
        if(times[i].is_null() || temps[i].is_null() || precips[i].is_null() || codes[i].is_null() || winds[i].is_null() || gusts[i].is_null()) {
            throw std::runtime_error("Null value found in hourly forecast data");
        }

        HourlyForecast hf;
        hf.time = times[i].get<std::string>();
        hf.conditions.temperature_c = temps[i].get<double>();
        hf.conditions.wind_kmh = winds[i].get<double>();
        hf.conditions.gusts_kmh = gusts[i].get<double>();
        hf.conditions.precip_probability_pct = precips[i].get<int>();
        hf.conditions.weather_code = codes[i].get<int>();
        hf.conditions.elevation_m = elev_m;

        std::string date_key = hf.time.size() >= 10 ? hf.time.substr(0, 10) : "";
        auto it = sunset_by_date.find(date_key);
        if(it != sunset_by_date.end()) {
            hf.sunset = it->second;
            hf.daylight_left_min = computeDaylightLeftMin(hf.time, hf.sunset);
        }

        result.push_back(hf);
    }

    return result;
}

std::vector<HourlyForecast> OpenMeteoClient::fetchForecast(double lat, double lon, std::optional<double> elevation_m, int forecast_days) const {
    if(!std::isfinite(lat) || lat < -90.0 || lat > 90.0) {
        throw std::invalid_argument("Latitude out of range [-90, 90]");
    }

    if(!std::isfinite(lon) || lon < -180.0 || lon > 180.0) {
        throw std::invalid_argument("Longitude out of range [-180, 180]");
    }

    if(forecast_days < 1 || forecast_days > 16) {
        throw std::invalid_argument("Forecast days out of range [1, 16]");
    }

    if(elevation_m.has_value()) {
        if(!std::isfinite(*elevation_m) || *elevation_m < -500 || *elevation_m > 9000.0) {
            throw std::invalid_argument("Elevation out of range [-500, 9000]");
        }
    }

    //Build URL path strictly from validated numbers
    std::ostringstream path;
    path << std::fixed << std::setprecision(4) << "/v1/forecast?latitude=" << lat << "&longitude=" << lon;

    if(elevation_m.has_value()) {
        path << std::setprecision(1) << "&elevation=" << *elevation_m;
    }

    path << "&timezone=Europe/Athens" << "&forecast_days=" << forecast_days << "&hourly=temperature_2m,apparent_temperature,precipitation_probability,weather_code,wind_speed_10m,wind_gusts_10m" << "&daily=sunrise,sunset";

    httplib::Client cli(host_);
    cli.set_connection_timeout(timeout_sec_, 0);
    cli.set_read_timeout(timeout_sec_, 0);
    cli.enable_server_certificate_verification(true);

    auto res = cli.Get(path.str());
    if(!res) {
        throw std::runtime_error("Failed to reach Open-Meteo (network error or timeout)");
    }
    if(res->status != 200) {
        throw std::runtime_error("Open-Meteo returned HTTP status " + std::to_string(res->status));
    }

    return parseResponse(res->body);
}
