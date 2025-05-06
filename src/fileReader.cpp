#include <fstream>
#include <string>

std::string extractFileContents(const std::string &fileName) {
    std::string fileText = "";
    std::ifstream file(fileName);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            fileText.append(line.append("\n"));
        }
        file.close();
    }
    return fileText;
}