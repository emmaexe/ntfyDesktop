#pragma once

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <QWidget>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui {
    class NotificationListItem;
}
QT_END_NAMESPACE

class NotificationListItem: public QWidget {
        Q_OBJECT
    public:
        NotificationListItem(
            const std::string& id, const std::string& server, const std::string& topic, const int& timestamp, const std::string& title, const std::string& message, const std::string& rawData,
            QWidget* parent = nullptr
        );
        ~NotificationListItem();
    signals:
        void sizeChanged(const QSize sizeHint);
    private:
        Ui::NotificationListItem* ui;
        std::string internalId, internalServer, internalTopic, internalTitle, internalMessage;
        int internalTimestamp;
};

class PixmapFetcher: public QObject {
        Q_OBJECT
    public:
        PixmapFetcher(std::string url, QObject* parent = nullptr);
    public slots:
        void fetchThumbnail();
    signals:
        void thumbnailFetched(const QPixmap pixmap);
    private:
        std::string url;
};

class AsyncCurlRequest: public QObject {
        Q_OBJECT
    public:
        AsyncCurlRequest(const nlohmann::json& action, QObject* parent = nullptr);
    public slots:
        void run();
    signals:
        void completed(bool success);
    private:
        static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        bool ready = false;
        CURL* handle;
        curl_slist* headers;
        std::string url, method, body;
        std::vector<std::string> rawHeaders;
};
