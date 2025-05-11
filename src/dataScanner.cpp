#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>

#include <core/dataScanner.h>

const int epochTime = 1900, monthOffset = 1;

// Curl thermal PNG received data write callback
size_t thermal_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::ofstream thermalImage;
    thermalImage.open("thermalImage.png", std::ios::out | std::ios::app | std::ios::binary);
    thermalImage.write(ptr, nmemb);
    thermalImage.close();
    return nmemb;
}

// Curl weather JSON recevied data write callback
size_t weather_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::ofstream thermalImage;
    thermalImage.open("weatherData.txt", std::ios::out | std::ios::app);
    thermalImage.write(ptr, nmemb);
    thermalImage.close();
    return nmemb;
}

void thermalData(tm date) {
    // Avoid fetching duplicate data
    std::ifstream thermalFile("thermalImage.png");
    if (thermalFile.good()) return;
    std::ostringstream oss;
    oss << "https://gibs.earthdata.nasa.gov/wms/epsg4326/best/wms.cgi?"
        << "SERVICE=WMS"
        << "&VERSION=1.3.0"
        << "&REQUEST=GetMap"
        << "&LAYERS=MODIS_Terra_Land_Surface_Temp_Day"
        << "&TIME="
        << (epochTime + date.tm_year) << "-"
        << std::setw(2) << std::setfill('0') << (monthOffset + date.tm_mon) << "-"
        << std::setw(2) << std::setfill('0') << date.tm_mday
        << "&CRS=EPSG:4326"
        << "&BBOX=-90,-180,90,180"
        << "&WIDTH=2048"
        << "&HEIGHT=1024"
        << "&FORMAT=image/png";
    std::string thermalLink = oss.str();
    CURL *curlHandle = curl_easy_init();
    if(curlHandle) {
        CURLcode res;
        curl_easy_setopt(curlHandle, CURLOPT_URL, thermalLink.c_str());
        curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &thermal_write_callback);
        res = curl_easy_perform(curlHandle);
        if (res == CURLE_OK) {
            std::cout << "Curl thermal query executed successfully." << std::endl;
        } else {
            std::cout << "Curl thermal query encountered an error: " << res << std::endl;
        }
        curl_easy_cleanup(curlHandle);
    }
}

void weatherData(double latitude, double longitude) {
    // Avoid fetching duplicate data
    std::ifstream weatherFile("weatherData.txt");
    if (weatherFile.good()) return;
    std::ostringstream oss;
    oss << "https://api.open-meteo.com/v1/forecast?"
        << "latitude=" 
        << std::fixed << std::setprecision(4) << latitude
        << "&longitude="
        << std::fixed << std::setprecision(4) << longitude
        << "&current=is_day,"
        << "apparent_temperature,"
        << "relative_humidity_2m,"
        << "temperature_2m,"
        << "precipitation,"
        << "rain,showers,"
        << "snowfall,"
        << "weather_code,"
        << "cloud_cover,"
        << "pressure_msl,"
        << "surface_pressure,"
        << "wind_speed_10m,"
        << "wind_gusts_10m,"
        << "wind_direction_10m";
    std::string weatherLink = oss.str();
    CURL *curlHandle = curl_easy_init();
    if(curlHandle) {
        CURLcode res;
        curl_easy_setopt(curlHandle, CURLOPT_URL, weatherLink.c_str());
        curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &weather_write_callback);
        res = curl_easy_perform(curlHandle);
        if (res == CURLE_OK) {
            std::cout << "Curl weather query executed successfully." << std::endl;
        } else {
            std::cout << "Curl weather query encountered an error: " << res << std::endl;
        }
        curl_easy_cleanup(curlHandle);
    }
}

void initializeData() {
    std::cout << "Scanning data sources." << std::endl;
    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);
    thermalData(datetime);
    weatherData(43.4675, -79.6877);
}