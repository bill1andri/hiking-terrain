#include "OpenMeteoClient.hpp"
#include "Cache.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

int main() {

    int failures = 0;

    //Read saved Open-Meteo response from disk (offline test)
    std::ifstream file("data/sample_response.json");
    if(!file) {
        std::cerr << "FAIL: cannot open data/sample_response.json\n";
        return 1;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();

    //Test 1: Parse valid 2-day forecast (48 hours)
    auto hours = OpenMeteoClient::parseResponse(buffer.str());
    if(hours.size() != 48) {
        std::cerr << "FAIL: expected 48 hourly entries, got " << hours.size() << "\n";
        failures++;
    }

    //Test 2: Verify parsed values and elevation on first entry
    if(std::abs(hours[0].conditions.elevation_m - 1026.0) > 0.1 ||
       std::abs(hours[0].conditions.temperature_c - 12.9) > 0.1) {
        std::cerr << "FAIL: unexpected values in hours[0]\n";
        failures++;
    }

    //Test 3: Verify daylight calculation at 09:00(sunset 17.09 -> 489 min) and 18:00(-> 0 min)
    if(hours[9].time != "2026-11-20T09:00" || hours[9].daylight_left_min != 489) {
        std::cerr << "FAIL: expected 489 daylight min at 09:00, got " << hours[9].daylight_left_min << "\n";
        failures++;
    }

    if(hours[18].daylight_left_min != 0) {
        std::cerr << "FAIL: expected 0 daylight min after sunset, got " << hours[18].daylight_left_min << "\n";
        failures++;
    }

    //Test 4: Missing field in JSON must be rejected
    bool threw_missing = false;
    try {
        OpenMeteoClient::parseResponse(R"({"elevation": 1026.0})");
    } catch (const std::exception&) {
        threw_missing = true;
    }
    if(!threw_missing) {
        std::cerr << "FAIL: expected exception when 'hourly' field is missing\n";
        failures++;
    }

    //Test 5: Cache hit on 2-decimal rounded coordinates and TTL expiration
    ForecastCache cache(std::chrono::minutes(30));
    cache.put(37.941, 23.824, 1026.0, hours);
    auto hit = cache.get(37.939, 23.818, 1026.0); //Both round to 37.94, 23.82
    if(!hit.has_value() || hit->size() != 48) {
        std::cerr << "FAIL: expected cache hit for nearby rounded coordinates\n";
        failures++;
    } 

    ForecastCache expired_cache(std::chrono::seconds(0));
    expired_cache.put(37.94, 23.82, 1026.0, hours);
    if(expired_cache.get(37.94, 23.82, 1026.0).has_value()) {
        std::cerr << "FAIL: expected cache miss when TTL is 0s\n";
        failures++;
    }

    if(failures == 0) {
        std::cout << "All JSON parse tests passed!\n";
        return 0;
    }

    return 1;
}