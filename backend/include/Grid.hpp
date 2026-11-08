#pragma once
#include<vector>

class Grid{
private:
    int _rows;
    int _cols;
    double _cellsize;//distance in m between 2 neighbouring points
    //simplification Dx=Dy=cellsize 
    std::vector<double> _data;
    //index func based on index=r*cols+c,index(r,c)
    int index(int r, int c) const{
        return r*_cols+c;
    }
    void check_bounds(int r, int c) const;
public:
    //constr
    Grid(int rows,int cols, double cellsize);
    //seters,getters
    double at(int r, int c) const;
    void set(int r, int c, double h);
    int rows() const{ 
        return _rows;
    }
    int cols() const{
        return _cols;
    }
    double cellsize() const{
        return _cellsize;
    }
    //destr
    ~Grid() = default;
};

