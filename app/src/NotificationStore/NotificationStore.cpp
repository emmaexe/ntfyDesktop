#include "NotificationStore.hpp"

#include "../Config/Config.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include <QCryptographicHash>
#include <QStandardPaths>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

bool NotificationStore::initialized = false;
std::map<std::string, std::string> NotificationStore::internalData = {};

void NotificationStore::update(const std::string& domain, const std::string& topic, const std::string& notificationID) {
    if (!NotificationStore::initialized) { NotificationStore::read(); }

    std::string hash = QCryptographicHash::hash(QString::fromStdString(domain + "/" + topic).toUtf8(), QCryptographicHash::Sha256).toHex().toStdString();
    NotificationStore::internalData[hash] = notificationID;
    NotificationStore::write();
}

std::optional<std::string> NotificationStore::get(const std::string& domain, const std::string& topic) {
    if (!NotificationStore::initialized) { NotificationStore::read(); }

    std::string hash = QCryptographicHash::hash(QString::fromStdString(domain + "/" + topic).toUtf8(), QCryptographicHash::Sha256).toHex().toStdString();
    if (NotificationStore::internalData.count(hash) == 0) { return std::nullopt; }
    return NotificationStore::internalData[hash];
}

bool NotificationStore::exists(const std::string& domain, const std::string& topic) { return NotificationStore::get(domain, topic).has_value(); }

void NotificationStore::remove(const std::string& domain, const std::string& topic) {
    if (!NotificationStore::initialized) { NotificationStore::read(); }

    std::string hash = QCryptographicHash::hash(QString::fromStdString(domain + "/" + topic).toUtf8(), QCryptographicHash::Sha256).toHex().toStdString();
    if (NotificationStore::internalData.count(hash) != 0) {
        NotificationStore::internalData.erase(hash);
        NotificationStore::write();
    }
}

void NotificationStore::configSync() {
    if (!NotificationStore::initialized) { NotificationStore::read(); }
    std::vector<std::string> hashes = {};
    for (nlohmann::json source: Config::data()["sources"]) {
        try {
            std::string urlpart = std::string(source["domain"]) + "/" + std::string(source["topic"]);
            hashes.push_back(QCryptographicHash::hash(QString::fromStdString(urlpart).toUtf8(), QCryptographicHash::Sha256).toHex().toStdString());
            if (!NotificationStore::exists(std::string(source["domain"]), std::string(source["topic"]))) {
                NotificationStore::update(std::string(source["domain"]), std::string(source["topic"]), "all");
            }
        } catch (nlohmann::json::out_of_range e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
    }

    for (auto iter = NotificationStore::internalData.begin(); iter != NotificationStore::internalData.end();) {
        if (std::find(hashes.begin(), hashes.end(), iter->first) == hashes.end()) {
            iter = NotificationStore::internalData.erase(iter);
        } else {
            iter++;
        }
    }

    NotificationStore::write();
}

void NotificationStore::read() {
    if (!std::filesystem::exists(NotificationStore::getStorePath()) || !std::filesystem::is_directory(NotificationStore::getStorePath())) {
        std::filesystem::create_directory(NotificationStore::getStorePath());
        return;
    }
    if (!std::filesystem::exists(NotificationStore::getStoreFile())) { return; }

    std::ifstream stream(NotificationStore::getStoreFile(), std::ios::in | std::ios::binary);
    if (stream.is_open()) {
        try {
            boost::archive::binary_iarchive bindata(stream);
            NotificationStore::internalData.clear();
            bindata >> NotificationStore::internalData;
        } catch (const std::exception& e) { std::cerr << "Error reading NotificationStore: " << e.what() << std::endl; }
    }
    stream.close();
}

void NotificationStore::write() {
    if (!std::filesystem::exists(NotificationStore::getStorePath()) || !std::filesystem::is_directory(NotificationStore::getStorePath())) {
        std::filesystem::create_directory(NotificationStore::getStorePath());
    }

    std::ofstream stream(NotificationStore::getStoreFile(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (stream.is_open()) {
        try {
            boost::archive::binary_oarchive bindata(stream);
            bindata << NotificationStore::internalData;
        } catch (const std::exception& e) { std::cerr << "Error writing NotificationStore: " << e.what() << std::endl; }
    }
    stream.close();
}

const std::string NotificationStore::getStorePath() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString(); }

const std::string NotificationStore::getStoreFile() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + std::string("/NotificationStore.bin"); }
