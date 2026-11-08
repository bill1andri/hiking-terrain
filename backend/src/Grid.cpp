#include<vector>
#include <stdexcept>
#include "Grid.hpp"
static void validation(int r, int c, double cs) { 
    if(r <= 0 || c<=0 || cs<=0.0){
        throw std::invalid_argument("Grid: rows, cols and cellsize must be positive");
    }
}
void Grid::check_bounds(int r, int c) const{ 
    if(r < 0 || r>=_rows || c < 0 || c >=_cols) { 
       throw std::out_of_range("Grid: (r, c) out of range");
    }
}
Grid::Grid(int rows, int cols, double cellsize):_rows(rows),_cols(cols),_cellsize(cellsize){
    validation(rows, cols,cellsize);
    _data.assign(rows*cols, 0.0);
}

double Grid::at(int r, int c) const { 
    check_bounds(r, c);
    return  _data[index(r,c)];
}
void Grid::set(int r, int c, double h) {
    check_bounds(r, c);
    _data[index(r,c)] = h;
}