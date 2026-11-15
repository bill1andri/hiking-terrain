// CLI entry point for Phase 1: reads weather scenarios from a CSV file and prints safety assessments.
#include "Atmosphere.hpp"
#include "Conditions.hpp"
#include "SafetyRules.hpp"
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    // Use the file path provided on the command line, or default to weather/data/scenarios.csv.
    std::string path = (argc > 1) ? argv[1] : "weather/data/scenarios.csv";
    std::ifstream file(path);
    if(!file) {
        std::cerr << "Error: cannot open file " << path << "\n";
        return 1;
    }

    // Mandatory user disclaimer: verdicts are an advisory aid, not a safety guarantee.
    std::cout << "Note: This verdict is an advisory aid, not a guarantee of safety.\n\n";
    
    // Process the CSV file one line (one scenario) at a time.
    std::string line;
    while(std::getline(file, line)) {
        if(line.empty()) continue;

        // Split the CSV line into individual comma-separated fields.
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> fields;
        while(std::getline(ss, cell, ',')) {
            fields.push_back(cell);
        }

        // Fail-safe check: never output GO if any of the 10 expected columns are missing.
        if(fields.size() < 10) {
            std::string name = fields.empty() ? "Uknown" : fields[0];
            std::cout << "Scenario: " << name << "\n" << "  Verdict: CAUTION\n" << "  Reason: Incomplete data\n\n";
            continue;
        }

        // Parse numeric values, compute atmospheric adjustments, and evaluate safety rules.
        try {
            std::string name = fields[0];
            Conditions c;
            c.temperature_c = std::stod(fields[1]);
            c.wind_kmh = std::stod(fields[2]);
            c.gusts_kmh = std::stod(fields[3]);
            c.precip_probability_pct = std::stoi(fields[4]);
            c.weather_code = std::stoi(fields[5]);
            c.elevation_m = std::stod(fields[6]);
            
            double target_elev_m = std::stod(fields[7]);
            int daylight_min = std::stoi(fields[8]);
            int duration_min = std::stoi(fields[9]);

            // Compute temperature and wind chill at target elevation, then run all safety rules.
            double temp_at_elev = temperatureAtElevation(c.temperature_c, c.elevation_m, target_elev_m);
            double feels_like = windChill(temp_at_elev, c.wind_kmh);
            Assessment res = evaluateSafety(c, target_elev_m, daylight_min, duration_min);

            // Print formatted summary (1 decimal place) and all triggered warning reasons.
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "Scenario: " << name << " (target: " << target_elev_m << " m)\n" 
                      << "  Temperature at elevation: " << temp_at_elev << " C\n"
                      << "  Feels like (wind chill):  " << feels_like << " C\n"
                      << "  Verdict: " << toString(res.verdict) << "\n";
            
            for(const std::string& reason : res.reasons) {
                std::cout << "  - " << reason << "\n";
            }
            std::cout << "\n";
        } catch (const std::exception& e) {
            // Catch and report invalid numeric strings or out-of-range physical values without crashing.
            std::cerr << "Rejected invalid scenario line: " << e.what() << "\n\n";
        }
    }

    return 0;
}