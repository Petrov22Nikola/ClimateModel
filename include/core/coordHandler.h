#pragma once

#include <unordered_map>

struct Coords {
    double latitude;
    double longitude;
};

// Generate mapping between city names and latitude / longitude coordinates
std::unordered_map<std::string, Coords> initCityCoords();