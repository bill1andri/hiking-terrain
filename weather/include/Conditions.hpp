#pragma once

// Weather conditions measured or forecast at a specific reference elevation.
struct Conditions {
    double temperature_c = 0.0;       // Air temperature in Celsius
    double wind_kmh = 0.0;            // Sustained wind speed in km/h
    double gusts_kmh = 0.0;           // Wind gusts in km/h
    int precip_probability_pct = 0;   // Precipitation probability (0-100%)
    int weather_code = 0;             // WMO weather interpretation code
    double elevation_m = 0.0;         // Reference elevation in meters
};