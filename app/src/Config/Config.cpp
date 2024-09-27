#include "Config.hpp"

#include "../NotificationStore/NotificationStore.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <QFile>
#include <QStandardPaths>
#include <filesystem>
#include <fstream>
#include <iostream>

bool Config::initialized = false;
bool Config::ok = true;
bool Config::updating = false;
nlohmann::json Config::internalData = {};
std::string Config::internalError = "";
int Config::internalErrorCounter = 0;

nlohmann::json& Config::data() {
    if (!Config::initialized && !Config::updating) { Config::read(); }
    return Config::internalData;
}

void Config::read() {
    Config::ok = true;
    Config::internalError = "";
    if (!std::filesystem::exists(Config::getConfigPath()) || !std::filesystem::is_directory(Config::getConfigPath())) { std::filesystem::create_directory(Config::getConfigPath()); }
    std::ifstream configStream(Config::getConfigFile());
    if (configStream.is_open()) {
        try {
            Config::internalData.update(nlohmann::json::parse(configStream));
        } catch (nlohmann::json::parse_error e) {
            Config::ok = false;
            Config::internalError = e.what();
        }
    } else {
        if (Config::internalErrorCounter <= 3) {
            internalErrorCounter++;
            return Config::reset();
        } else {
            Config::ok = false;
            Config::internalError = "The config file does not exist.";
        }
    }
    configStream.close();

    if (!Config::updating) {
        Config::updateToCurrent();
        Config::initialized = true;
        Config::internalErrorCounter = 0;
        NotificationStore::configSync();
    }
}

void Config::write() {
    if (!Config::initialized && !Config::updating) { Config::read(); }
    if (!std::filesystem::exists(Config::getConfigPath()) || !std::filesystem::is_directory(Config::getConfigPath())) { std::filesystem::create_directory(Config::getConfigPath()); }
    std::string configSerialized = Config::internalData.dump();
    std::ofstream configStream(Config::getConfigFile(), std::ios::trunc);
    configStream << configSerialized;
    configStream.close();
    if (!Config::updating) { NotificationStore::configSync(); }
}

bool Config::ready() {
    if (!Config::initialized && !Config::updating) { Config::read(); }
    return Config::ok;
}

void Config::reset() {
    while (!QFile::copy(":/config/defaultConfig.json", QString::fromStdString(Config::getConfigFile()))) { QFile::remove(QString::fromStdString(Config::getConfigFile())); }
    QFile(QString::fromStdString(Config::getConfigFile())).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    Config::read();
}

const std::string& Config::getError() {
    if (!Config::initialized && !Config::updating) { Config::read(); }
    return Config::internalError;
}

void Config::updateToCurrent() {
    if (Config::updating) { return; }
    Config::updating = true;
    std::string version = Config::internalData["version"];
    if (Util::versionCompare(ND_VERSION, version) == 0) {
        Config::updating = false;
        return;
    } else if (Util::versionCompare(ND_VERSION, version) > 0) {
        Config::ok = false;
        Config::internalError = "The config (" + Config::getConfigFile() + ") was created for a newer version of ntfyDesktop. Please downgrade your config manually, reset it to default values or update ntfyDesktop.";
    } else if (Util::versionCompare(version, "1.0.0") >= 0) {
        // Version is 1.0.0 or older
        // Schema used is: {"version": string, "sources": [{"name": string, "server": string, "topic": string}, ...]}

        nlohmann::json data = Config::internalData;
        while (!QFile::copy(QString::fromStdString(Config::getConfigFile()), QString::fromStdString(Config::getConfigFile() + ".bak"))) {
            QFile::remove(QString::fromStdString(Config::getConfigFile() + ".bak"));
        }
        Config::reset();

        for (nlohmann::json source: data["sources"]) {
            nlohmann::json newSource = {};
            newSource["name"] = source["name"];
            newSource["domain"] = source["server"];
            newSource["topic"] = source["topic"];
            newSource["secure"] = true;
            Config::data()["sources"].push_back(newSource);
        }

        Config::write();
    }
    Config::updating = false;
}

const std::string Config::getConfigPath() { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString() + std::string("/moe.emmaexe.ntfyDesktop/"); }

const std::string Config::getConfigFile() { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString() + std::string("/moe.emmaexe.ntfyDesktop/config.json"); }
