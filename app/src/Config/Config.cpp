#include "Config.hpp"

#include <QStandardPaths>
#include <filesystem>
#include <fstream>
#include <iostream>

bool Config::initialized = false;
bool Config::ok = true;
nlohmann::json Config::internalData = {};
std::string Config::internalError = "";

nlohmann::json& Config::data() {
    if (!Config::initialized) { Config::read(); }
    return Config::internalData;
}

void Config::read() {
    Config::ok = true;
    Config::internalError = "";
    std::ifstream configStream(Config::getConfigFile());
    if (configStream.is_open()) {
        try {
            Config::internalData.update(nlohmann::json::parse(configStream));
        } catch (nlohmann::json::parse_error e) {
            Config::ok = false;
            Config::internalError = e.what();
        }
    } else {
        Config::ok = false;
        Config::internalError = "The config file does not exist.";
    }
    configStream.close();
    Config::initialized = true;
}

void Config::write() {
    if (!Config::initialized) { Config::read(); }
    if (!std::filesystem::exists(Config::getConfigPath()) || !std::filesystem::is_directory(Config::getConfigPath())) { std::filesystem::create_directory(Config::getConfigPath()); }
    std::string configSerialized = Config::internalData.dump();
    std::ofstream configStream(Config::getConfigFile(), std::ios::trunc);
    configStream << configSerialized;
    configStream.close();
}

bool Config::ready() {
    if (!Config::initialized) { Config::read(); }
    return Config::ok;
}

const std::string& Config::getError() {
    if (!Config::initialized) { Config::read(); }
    return Config::internalError;
}

const std::string Config::getConfigPath() { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString() + std::string("/ntfyDesktop/"); }

const std::string Config::getConfigFile() { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString() + std::string("/ntfyDesktop/config.json"); }
