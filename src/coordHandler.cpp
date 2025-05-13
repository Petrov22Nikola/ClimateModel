#include <iostream>
#include <string>
#include <utility>
#include <unordered_map>
#include <core/fileReader.h>
#include <core/coordHandler.h>

const std::string filePath = __FILE__;

std::unordered_map<std::string, Coords> initCityCoords() {
    std::unordered_map<std::string, Coords> cityMap;
    std::string dataPath = filePath + "\\..\\..\\data\\worldcities.csv";
    std::string contents = extractFileContents(dataPath);
    bool openBracket = false, init = true;
    int numBrackets = 0;
    std::string input = "", city = "", latitude = "", longitude = "";
    for (int i = 0; i < contents.size(); ++i) {
        char c = contents[i];
        if (init && c != '\n') continue;
        else if (init) {
            init = false;
            continue;
        }
        switch (c) {
            case '"': {
                openBracket = !openBracket;
                if (openBracket) input.clear();
                ++numBrackets;
                if (numBrackets == 2) city = input;
                else if (numBrackets == 6) latitude = input;
                else if (numBrackets == 8) longitude = input;
                break;
            }
            case '\n': {
                Coords cityCoords = {std::stod(latitude), std::stod(longitude)};
                cityMap[city] = cityCoords;
                numBrackets = 0;
            }
            default: {
                input.push_back(c);
            }
        }
    }
    return cityMap;
}