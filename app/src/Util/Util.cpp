#include "Util.hpp"

#include <algorithm>
#include <cstdlib>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

size_t stringWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* rawData = static_cast<std::string*>(userdata);
    rawData->append(ptr, size*nmemb);
    return size*nmemb;
}

namespace Util {
    int random(int min, int max) {
        return rand() % (max - min + 1) + min;
    }
    std::string getRandomUA(int limit) {
        std::string rawData;
        CURL* curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/microlinkhq/top-user-agents/master/src/index.json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stringWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawData);
        if (curl_easy_perform(curl) != CURLE_OK) { return "curl/" + std::string(curl_version()); }
        try {
            nlohmann::json data = nlohmann::json::parse(rawData);
            int target = Util::random(0, std::min(limit, static_cast<int>(data.size())));
            return data.at(target);
        } catch(nlohmann::json::parse_error e) {
            return "curl/" + std::string(curl_version());
        }
    }

}
