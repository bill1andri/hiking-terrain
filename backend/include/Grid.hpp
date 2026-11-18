#pragma once
#include<vector>

class Grid{
private:
    int _rows;
    int _cols;
    double _dx;//distance in m between 2 neighbouring columns (east-west)
    double _dy;//distance in m between 2 neighbouring rows (north-south)
    std::vector<double> _data;
    //index func based on index=r*cols+c,index(r,c)
    int index(int r, int c) const{
        return r*_cols+c;
    }
    void check_bounds(int r, int c) const;
public:
    //constr
    Grid(int rows, int cols, double dx, double dy);
    //square cells: delegates to the main constructor with dx = dy = cellsize
    Grid(int rows, int cols, double cellsize);
    //seters,getters
    double at(int r, int c) const;
    void set(int r, int c, double h);
    int rows() const{ 
        return _rows;
    }
    int cols() const{
        return _cols;
    }
    double dx() const{
        return _dx;
    }
    double dy() const{
        return _dy;
    }
    //destr
    ~Grid() = default;
};

