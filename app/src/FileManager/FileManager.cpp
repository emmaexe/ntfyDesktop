#include "FileManager.hpp"

#include <QApplication>
#include <curl/curl.h>
#include <fstream>

#include "FileManagerException.hpp"

std::vector<QTemporaryFile*> FileManager::tempFileHolder = {};

QUrl FileManager::urlToTempFile(QUrl url) {
    QTemporaryFile* file = new QTemporaryFile(QApplication::instance());
    if (!file->open()) {
        throw FileManagerException("Unable to create temporary file.");
    }
    file->setAutoRemove(true);
    FileManager::tempFileHolder.push_back(file);
    std::ofstream fileStream(file->fileName().toStdString(), std::ios::binary);
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.toString().toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileManager::urlToTempFileWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileStream);
        // TODO: Random user agent

        char curlError[CURL_ERROR_SIZE] = "";
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlError);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::string err = "Failed to download file: ";
            err.append(curlError);
            throw FileManagerException(err.c_str());
        }
    } else {
        throw FileManagerException("Unable to create libcurl handle.");
    }
    return QUrl::fromLocalFile(file->fileName());

}

size_t FileManager::urlToTempFileWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ofstream* fileStream = static_cast<std::ofstream*>(userdata);
    fileStream->write(ptr, size*nmemb);
    return size*nmemb;
}
