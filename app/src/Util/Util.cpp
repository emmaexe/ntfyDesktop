#include "Util.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <QApplication>
#include <QCryptographicHash>
#include <QSpacerItem>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <regex>
#include <stdexcept>

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

    bool isTopic(const std::string& topic) { return std::regex_match(topic, std::regex("^[-0-9A-Z_a-z]{1,64}$")); }

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

    int versionCompare(const std::string& first, const std::string& second) {
        std::vector<std::string> firstParts = split(first, ".");
        std::vector<std::string> secondParts = split(second, ".");
        if (firstParts.size() != 3) { throw std::invalid_argument("Could not parse version: " + first); }
        if (secondParts.size() != 3) { throw std::invalid_argument("Could not parse version: " + second); }

        int firstMajor, firstMinor, firstPatch, secondMajor, secondMinor, secondPatch;
        try {
            firstMajor = std::stoi(firstParts[0]);
            firstMinor = std::stoi(firstParts[1]);
            firstPatch = std::stoi(firstParts[2]);
        } catch (const std::invalid_argument& e) { throw std::invalid_argument("Could not parse version, NaN: " + first); } catch (const std::out_of_range& e) {
            throw std::invalid_argument("Version number is too large: " + first);
        }
        try {
            secondMajor = std::stoi(secondParts[0]);
            secondMinor = std::stoi(secondParts[1]);
            secondPatch = std::stoi(secondParts[2]);
        } catch (const std::invalid_argument& e) { throw std::invalid_argument("Could not parse version, NaN: " + second); } catch (const std::out_of_range& e) {
            throw std::invalid_argument("Version number is too large: " + second);
        }

        if (firstMajor != secondMajor) {
            return firstMajor > secondMajor ? -1 : 1;
        } else if (firstMinor != secondMinor) {
            return firstMinor > secondMinor ? -1 : 1;
        } else if (firstPatch != secondPatch) {
            return firstPatch > secondPatch ? -1 : 1;
        } else {
            return 0;
        }
    }

    namespace Colors {
        const QColor colorShiftSuccess(const QColor& base) {
            float h, s, l, a;
            base.getHslF(&h, &s, &l, &a);

            h = 120.0 / 360.0; // Hue for green is 120°

            s = std::clamp(static_cast<float>(s * 1.2), 0.5F, 1.0F);

            l *= (l > 0.5) ? 0.85 : 1.15;
            l = std::clamp(l, 0.35F, 0.65F);

            QColor color;
            color.setHslF(h, s, l, a);
            return color;
        }

        const QColor colorShiftFailure(const QColor& base) {
            float h, s, l, a;
            textColor().getHslF(&h, &s, &l, &a);

            h = 0.0; // Hue for red is 0°

            s = std::clamp(static_cast<float>(s * 1.2), 0.5F, 1.0F);

            l *= (l > 0.5) ? 0.85 : 1.15;
            l = std::clamp(l, 0.35F, 0.65F);

            QColor color;
            color.setHslF(h, s, l, a);
            return color;
        }

        const QColor textColor() { return QApplication::palette().color(QPalette::WindowText); }
        const QColor textColorSuccess() { return colorShiftSuccess(textColor()); }
        const QColor textColorFailure() { return colorShiftFailure(textColor()); }
        const QColor buttonColor() { return QApplication::palette().color(QPalette::Button); }
        const QColor buttonColorSuccess() { return colorShiftSuccess(buttonColor()); }
        const QColor buttonColorFailure() { return colorShiftFailure(buttonColor()); }
        const QColor buttonTextColor() { return QApplication::palette().color(QPalette::ButtonText); }
        const QColor buttonTextColorSuccess() { return buttonTextColor(); }
        const QColor buttonTextColorFailure() { return buttonTextColor(); }
    }

    std::string topicHash(const std::string& domain, const std::string& topic) {
        return QCryptographicHash::hash(QString::fromStdString(domain + "/" + topic).toUtf8(), QCryptographicHash::Sha256).toHex().toStdString();
    }

    std::string timeToString(const std::time_t& time) {
        std::ostringstream stream;
        stream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    void toUpper(std::string& str) {
        for (char& c: str) { c = toupper(c); }
    }

    void toLower(std::string& str) {
        for (char& c: str) { c = tolower(c); }
    }
}
