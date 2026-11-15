#pragma once

// Adjusts temperature for target elevation using the standard lapse rate (-6.5 C per 1000 m).
double temperatureAtElevation(double temp_ref_c, double elev_ref_m, double target_elev_m);

// Computes perceived wind-chill temperature in Celsius (active when T <= 10 C and wind > 4.8 km/h).
double windChill(double temp_c, double wind_kmh);