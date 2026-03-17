#include "FileManager.hpp"

#include "../Util/Curl.hpp"
#include "./Util.hpp"
#include "ntfyDesktop.hpp"

#include <curl/curl.h>
#include <unistd.h>

#include <QApplication>
#include <fstream>
#include <tuple>

FileManager& FileManager::instance() {
    static FileManager* instance = new FileManager(QApplication::instance());
    return *instance;
}

std::expected<QUrl, std::string> FileManager::url_to_temp_file(QUrl url, bool outsidePath) {
    this->mutex.lock();
    auto target = this->files.find(url);
    bool found = target != this->files.end();
    this->mutex.unlock();

    if (found) {
        target->second.first->lock();
        QString fileName = target->second.second->fileName();
        target->second.first->unlock();
        if (ND_BUILD_TYPE == "Flatpak" && outsidePath) { fileName.prepend(QString::fromStdString("/run/user/" + std::to_string(getuid()) + "/.flatpak/moe.emmaexe.ntfyDesktop")); }
        return QUrl::fromLocalFile(fileName);
    }

    QTemporaryFile* file = new QTemporaryFile(this);
    this->mutex.lock();
    auto [iterator, inserted] = this->files.emplace(url, std::make_pair(std::make_unique<std::mutex>(), file));
    this->mutex.unlock();

    std::lock_guard<std::mutex> guard(*iterator->second.first);

    if (!file->open()) { return std::unexpected("Unable to create temporary file."); }
    file->setAutoRemove(true);

    std::ofstream fileStream(file->fileName().toStdString(), std::ios::binary);
    Curl curlInstance = Curl::withDefaults();

    curlInstance.setOpt(CURLOPT_URL, url.toString().toStdString().c_str());
    curlInstance.setOpt(CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        std::ofstream* fileStream = static_cast<std::ofstream*>(userdata);
        fileStream->write(ptr, size * nmemb);
        return size * nmemb;
    });
    curlInstance.setOpt(CURLOPT_WRITEDATA, &fileStream);
    curlInstance.setOpt(CURLOPT_FOLLOWLOCATION, 1L);

    char curlError[CURL_ERROR_SIZE] = "";
    curlInstance.setOpt(CURLOPT_ERRORBUFFER, curlError);

    if (curl_easy_perform(curlInstance.handle()) != CURLE_OK) {
        return std::unexpected(std::format("Failed to download file: {}", curlError));
    }

    QString fileName = file->fileName();
    if (ND_BUILD_TYPE == "Flatpak" && outsidePath) { fileName.prepend(QString::fromStdString("/run/user/" + std::to_string(getuid()) + "/.flatpak/moe.emmaexe.ntfyDesktop")); }
    return QUrl::fromLocalFile(fileName);
}

FileManager::FileManager(QObject* parent): QObject(parent) {}
