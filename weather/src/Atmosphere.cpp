#include "Atmosphere.hpp"
#include <cmath>
#include <stdexcept>

// Standard atmospheric lapse rate: T(h) = T_ref - 0.0065 * (h - h_ref)
double temperatureAtElevation(double temp_ref_c, double elev_ref_m, double target_elev_m) {
    return (temp_ref_c - 0.0065*(target_elev_m - elev_ref_m));
}

// North American / Environment Canada wind chill index formula.
double windChill(double temp_c, double wind_kmh) {
    if(wind_kmh < 0.0) {
        throw std::invalid_argument("Wind speed cannot be negative");
    }

    // Wind chill formula only applies at low temperatures and noticeable wind speeds.
    if((temp_c <= 10.0) && (wind_kmh > 4.8)) {
        double v_pow = std::pow(wind_kmh, 0.16);

        return (13.12 + 0.6215*temp_c - 11.37*v_pow + 0.3965*temp_c*v_pow);
    }
    else {
        return temp_c;
    }
}