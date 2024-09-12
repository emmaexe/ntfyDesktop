#pragma once

#include <QUrl>
#include <QTemporaryFile>

#include "FileManagerException.hpp"

/**
 * @brief Static class with misc file managment functionality
 */
class FileManager {
    public:
        FileManager() = delete;
        /**
         * @brief Temporarly download a file from the web.
         *
         * @param url Url to a file on the web.
         * @return QUrl - Url to a temporary locally downloaded copy of the file from the web. The file will be deleted when the QApplication exits.
         */
        static QUrl urlToTempFile(QUrl url);
    private:
        static size_t urlToTempFileWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static std::vector<QTemporaryFile*> tempFileHolder;
};
