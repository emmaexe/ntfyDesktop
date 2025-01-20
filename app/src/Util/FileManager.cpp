#include "FileManager.hpp"

#include "./Util.hpp"
#include "ntfyDesktop.hpp"

#include <curl/curl.h>
#include <unistd.h>

#include <QApplication>
#include <fstream>
#include <tuple>

FileManagerException::FileManagerException(const char* message): message(message) {}

const char* FileManagerException::what() const throw() { return this->message.c_str(); }

std::map<QUrl, std::pair<std::unique_ptr<std::mutex>, QTemporaryFile*>> FileManager::tempFileHolder = {};
std::mutex FileManager::tempFileHolderLock = std::mutex();

void FileManager::init() {
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, []() { FileManager::cleanup(); });
}

QUrl FileManager::urlToTempFile(QUrl url, bool outsidePath) {
    FileManager::tempFileHolderLock.lock();
    auto target = FileManager::tempFileHolder.find(url);
    bool found = target != FileManager::tempFileHolder.end();
    FileManager::tempFileHolderLock.unlock();

    if (found) {
        target->second.first->lock();
        QString fileName = target->second.second->fileName();
        target->second.first->unlock();
        if (ND_BUILD_TYPE == "Flatpak" && outsidePath) { fileName.prepend(QString::fromStdString("/run/user/" + std::to_string(getuid()) + "/.flatpak/moe.emmaexe.ntfyDesktop")); }
        return QUrl::fromLocalFile(fileName);
    }

    QTemporaryFile* file = new QTemporaryFile();
    FileManager::tempFileHolderLock.lock();
    auto [iterator, inserted] = FileManager::tempFileHolder.emplace(url, std::make_pair(std::make_unique<std::mutex>(), file));
    FileManager::tempFileHolderLock.unlock();

    std::lock_guard<std::mutex> guard(*iterator->second.first);

    if (!file->open()) { throw FileManagerException("Unable to create temporary file."); }
    file->setAutoRemove(true);

    std::ofstream fileStream(file->fileName().toStdString(), std::ios::binary);
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.toString().toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileManager::urlToTempFileWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileStream);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, Util::getRandomUA().c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        char curlError[CURL_ERROR_SIZE] = "";
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlError);

        if (curl_easy_perform(curl) != CURLE_OK) {
            std::string err = "Failed to download file: ";
            err.append(curlError);
            throw FileManagerException(err.c_str());
        }
    } else {
        throw FileManagerException("Unable to create libcurl handle.");
    }

    QString fileName = file->fileName();
    if (ND_BUILD_TYPE == "Flatpak" && outsidePath) { fileName.prepend(QString::fromStdString("/run/user/" + std::to_string(getuid()) + "/.flatpak/moe.emmaexe.ntfyDesktop")); }
    return QUrl::fromLocalFile(fileName);
}

void FileManager::cleanup() {
    for (auto& [url, file]: FileManager::tempFileHolder) {
        if (file.second) {
            delete file.second;
            file.second = nullptr;
        }
    }
    FileManager::tempFileHolder.clear();
}

size_t FileManager::urlToTempFileWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ofstream* fileStream = static_cast<std::ofstream*>(userdata);
    fileStream->write(ptr, size * nmemb);
    return size * nmemb;
}
