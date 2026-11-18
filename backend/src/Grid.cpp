#include<vector>
#include <stdexcept>
#include <cmath>
#include "Grid.hpp"
static void validation(int r, int c, double dx, double dy) { 
    if(r <= 0 || c<=0 || !std::isfinite(dx) || dx<=0.0 || !std::isfinite(dy) || dy<=0.0){
        throw std::invalid_argument("Grid: rows, cols, dx and dy must be positive");
    }
}
void Grid::check_bounds(int r, int c) const{ 
    if(r < 0 || r>=_rows || c < 0 || c >=_cols) { 
       throw std::out_of_range("Grid: (r, c) out of range");
    }
}
Grid::Grid(int rows, int cols, double dx, double dy):_rows(rows),_cols(cols),_dx(dx),_dy(dy){
    validation(rows, cols, dx, dy);
    _data.assign(rows*cols, 0.0);
}
Grid::Grid(int rows, int cols, double cellsize):Grid(rows, cols, cellsize, cellsize){}

double Grid::at(int r, int c) const { 
    check_bounds(r, c);
    return  _data[index(r,c)];
}
void Grid::set(int r, int c, double h) {
    check_bounds(r, c);
    _data[index(r,c)] = h;
}