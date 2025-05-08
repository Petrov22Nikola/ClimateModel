#include <iostream>
#include <curl/curl.h>

#include <core/dataScanner.h>

void initializeData() {
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout << "Scanning data." << std::endl;
}