#include "GridIO.hpp"
#include "Grid.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>

// Μετατρέπει ένα κελί σε αριθμό, αλλιώς πετάει μήνυμα με τη γραμμή.
// static: υπάρχει μόνο μέσα σε αυτό το αρχείο.
static double parseCell(const std::string& cell, int lineNo) {
    const std::string error = "line " + std::to_string(lineNo) +
                              ": not a number: '" + cell + "'";
    std::size_t pos = 0;  // πόσους χαρακτήρες διάβασε το stod
    double v = 0.0;
    try {
        v = std::stod(cell, &pos);
    } catch (const std::exception&) {  // "abc", "" -> το stod πετάει μόνο του
        throw std::runtime_error(error);
    }
    if (pos != cell.size()) {           // "12abc" -> διάβασε μόνο το "12"
        throw std::runtime_error(error);
    }
    if (!std::isfinite(v)) {            // "nan", "inf" -> το stod τα δέχεται
        throw std::runtime_error(error);
    }
    return v;
}

Grid loadGridCsv(const std::string& path, double cellsize) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open file: " + path);
    }

    std::vector<double> values;
    std::string line;
    int rows = 0, cols = 0;

    while (std::getline(in, line)) {
        const int lineNo = rows + 1;  // οι άνθρωποι μετράνε από το 1
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();  // αρχεία από Windows τελειώνουν σε \r\n
        }
        int count = 0;
        std::stringstream ss(line);   // η γραμμή γίνεται stream
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            values.push_back(parseCell(cell, lineNo));
            count++;
        }

        if (rows == 0) {
            cols = count;  // η πρώτη γραμμή ορίζει το πλάτος
        } else if (count != cols) {
            throw std::runtime_error("line " + std::to_string(lineNo) +
                                     ": has " + std::to_string(count) +
                                     " values, expected " + std::to_string(cols));
        }
        rows++;
    }

    if (rows == 0) {
        throw std::runtime_error("empty file: " + path);
    }

    Grid g(rows, cols, cellsize);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            g.set(r, c, values[r * cols + c]);
        }
    }
    return g;
}