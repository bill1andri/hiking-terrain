#pragma once
#include "Grid.hpp"

struct Gradient { 
    double dx;//os pros x tou h 
    double dy;
};

Gradient gradientAt(const Grid& g, int r, int c);
double slopeDegrees(const Gradient& grad);
Grid slopeGrid(const Grid& heights); 