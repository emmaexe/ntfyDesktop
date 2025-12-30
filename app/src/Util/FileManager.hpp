#pragma once

#include <QObject>
#include <QTemporaryFile>
#include <QUrl>
#include <expected>
#include <mutex>
#include <string>

/**
 * @brief A singleton with misc file managment functionality
 */
class FileManager: public QObject {
        Q_OBJECT
        Q_DISABLE_COPY(FileManager)
    public:
        /**
         * @brief Get the singleton instance
         */
        static FileManager& instance();

        /**
         * @brief Temporarly download a file from the web.
         *
         * @param url Url to a file on the web.
         * @return QUrl - Url to a temporary locally downloaded copy of the file from the web. The file will be deleted when the QApplication exits.
         */
        std::expected<QUrl, std::string> url_to_temp_file(QUrl url, bool outsidePath = false);
    private:
        FileManager(QObject* parent = nullptr);
        std::map<QUrl, std::pair<std::unique_ptr<std::mutex>, QTemporaryFile*>> files;
        std::mutex mutex;
};
