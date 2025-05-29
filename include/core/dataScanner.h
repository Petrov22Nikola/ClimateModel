#pragma once

#include <core/coordHandler.h>

// Scan data sources for latest data
Coords initializeData(std::string location);

// Load locale weather data
void localeWeatherData(double apiLat, double apiLong);