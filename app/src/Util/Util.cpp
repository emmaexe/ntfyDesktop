#include "Util.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <QSpacerItem>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <regex>

size_t stringWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* rawData = static_cast<std::string*>(userdata);
    rawData->append(ptr, size * nmemb);
    return size * nmemb;
}

namespace Util {
    int random(int min, int max) { return rand() % (max - min + 1) + min; }

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
        } catch (nlohmann::json::parse_error e) { return "curl/" + std::string(curl_version()); }
    }

    std::vector<std::string> split(const std::string& string, const std::string& delimiter) {
        std::vector<std::string> parts;
        std::string string_c = std::string(string);
        size_t pos = 0;
        std::string token;
        while ((pos = string_c.find(delimiter)) != std::string::npos) {
            token = string_c.substr(0, pos);
            parts.push_back(token);
            string_c.erase(0, pos + delimiter.length());
        }
        parts.push_back(string_c);
        return parts;
    }

    bool isDomain(const std::string& domain) { return std::regex_match(domain, std::regex("^[.0-9:A-Za-z]+$")); }

    bool isTopic(const std::string& topic) { return std::regex_match(topic, std::regex("^[\-0-9A-Z_a-z]{1,64}$")); }

    void setLayoutVisibility(QLayout* layout, bool visible) {
        for (int i = 0; i < layout->count(); i++) {
            QWidget* widget = layout->itemAt(i)->widget();
            QLayout* subLayout = layout->itemAt(i)->layout();
            if (widget) {
                widget->setVisible(visible);
            } else if (subLayout) {
                Util::setLayoutVisibility(subLayout, visible);
            }
        }
    }
}
