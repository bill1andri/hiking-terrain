#pragma once
#include <cstdint>
#include <string>
#include <vector>

class DemTile{
private:
    int _size;
    double _latSouth,_lonWest;
    std::vector<std::int16_t> _data;
public:
    DemTile(int s, double ls, double lw, std::vector<std::int16_t> d);
    int size() { 
        return _size;
    }
    double latSouth() { 
        return _latSouth;
    }
    double lonWest(){
        return _lonWest;
    }
    std::int16_t at(int r, int c) const;
};

DemTile loadHgt(const std::string& path, double latSouth, double lonWest);