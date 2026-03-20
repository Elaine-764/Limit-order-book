#include "../include/lob/utilities.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <stdexcept>
#include <format>

// curl needs a static callback to write response into a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

Price find_last_price(const std::string& ticker) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl init failed");

    std::string readBuffer;
    std::string url = std::format(
        "https://query1.finance.yahoo.com/v8/finance/chart/{}?interval=1d", 
        ticker
    );

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());   // c_str() here
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0"); // yahoo blocks default curl agent

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::format("curl failed: {}", curl_easy_strerror(res)));
    }

    // parse JSON
    try {
        auto json = nlohmann::json::parse(readBuffer);
        double price = json["chart"]["result"][0]["meta"]["regularMarketPrice"];
        
        // convert to integer ticks (multiply by 100 to preserve cents)
        return static_cast<Price>(price * 100);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::format("JSON parse failed: {}", e.what()));
    }
}