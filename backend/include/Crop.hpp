#pragma once
#include "Grid.hpp"
#include "DemLoader.hpp"

Grid cropToGrid(const DemTile& DT, double latMin, double latMax, double lonMin, double lonMax);