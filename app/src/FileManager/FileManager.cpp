#include "FileManager.hpp"

#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <curl/curl.h>
#include <unistd.h>

#include <QApplication>
#include <fstream>

std::vector<QTemporaryFile*> FileManager::tempFileHolder = {};

QUrl FileManager::urlToTempFile(QUrl url) {
    QTemporaryFile* file = new QTemporaryFile(QApplication::instance());
    if (!file->open()) { throw FileManagerException("Unable to create temporary file."); }
    file->setAutoRemove(true);
    FileManager::tempFileHolder.push_back(file);
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
    // if (ND_BUILD_TYPE == "Flatpak") { fileName.prepend(QString::fromStdString("/run/user/" + std::to_string(getuid()) + "/.flatpak/moe.emmaexe.ntfyDesktop")); }
    return QUrl::fromLocalFile(fileName);
}

size_t FileManager::urlToTempFileWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ofstream* fileStream = static_cast<std::ofstream*>(userdata);
    fileStream->write(ptr, size * nmemb);
    return size * nmemb;
}
