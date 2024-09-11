#pragma once

#include <QUrl>
#include <QTemporaryFile>

class FileManager {
    public:
        FileManager() = delete;
        static QUrl urlToTempFile(QUrl url);
    private:
        static size_t urlToTempFileWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static std::vector<QTemporaryFile*> tempFileHolder;
};
