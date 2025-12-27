#pragma once

#include <QTemporaryFile>
#include <QUrl>
#include <exception>
#include <mutex>
#include <string>

class FileManagerException: public std::exception {
    public:
        FileManagerException(std::string_view message);
        const char* what() const throw();
    private:
        std::string message;
};

/**
 * @brief Static class with misc file managment functionality
 */
class FileManager {
    public:
        FileManager() = delete;
        /**
         * @brief This function initializes FileManager (eager loading).
         * If it is not called at the start of the program, lazy loading will be used instead.
         */
        static void init();
        /**
         * @brief Temporarly download a file from the web.
         *
         * @param url Url to a file on the web.
         * @return QUrl - Url to a temporary locally downloaded copy of the file from the web. The file will be deleted when the QApplication exits.
         */
        static QUrl urlToTempFile(QUrl url, bool outsidePath = false);
    private:
        inline static std::once_flag init_flag;
        static void init_impl() noexcept;
        static void cleanup();
        static size_t urlToTempFileWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static std::map<QUrl, std::pair<std::unique_ptr<std::mutex>, QTemporaryFile*>> tempFileHolder;
        static std::mutex tempFileHolderLock;
};
