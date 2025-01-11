#include "NotificationListItem.hpp"

#include "../NotificationManager/NtfyNotification.hpp"
#include "../Util/FileManager.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"
#include "ui_NotificationListItem.h"

#include <QDesktopServices>
#include <QListWidgetItem>
#include <QPushButton>
#include <QThread>

NotificationListItem::NotificationListItem(
    const std::string& id, const std::string& server, const std::string& topic, const int& timestamp, const std::string& title, const std::string& message, const std::string& rawData, QWidget* parent
):
    QWidget(parent),
    ui(new Ui::NotificationListItem),
    internalId(id),
    internalServer(server),
    internalTopic(topic),
    internalTimestamp(timestamp),
    internalTitle("<b>" + title + "</b>"),
    internalMessage(message) {
    this->ui->setupUi(this);
    this->ui->titleLabel->setText(QString::fromStdString(this->internalTitle));
    this->ui->messageLabel->setText(QString::fromStdString(this->internalMessage));
    this->ui->timestampLabel->setText(QString::fromStdString(Util::timeToString(static_cast<std::time_t>(this->internalTimestamp))));
    this->ui->serverTopicLabel->setText(QString::fromStdString(this->internalServer + "/" + this->internalTopic));

    try {
        nlohmann::json notificationData = nlohmann::json::parse(rawData);
        if (notificationData.contains("icon")) {
            QThread* thread = new QThread;
            PixmapFetcher* pixmapFetcher = new PixmapFetcher(static_cast<std::string>(notificationData["icon"]));
            pixmapFetcher->moveToThread(thread);

            connect(thread, &QThread::started, pixmapFetcher, &PixmapFetcher::fetchThumbnail);
            connect(
                pixmapFetcher,
                &PixmapFetcher::thumbnailFetched,
                this,
                [this](const QPixmap pixmap) {
                    this->ui->thumbnail->setPixmap(pixmap);
                    emit this->sizeChanged(this->sizeHint());
                },
                Qt::QueuedConnection
            );

            connect(pixmapFetcher, &PixmapFetcher::thumbnailFetched, pixmapFetcher, &PixmapFetcher::deleteLater);
            connect(pixmapFetcher, &PixmapFetcher::thumbnailFetched, thread, &QThread::quit);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);

            thread->start();
        }

        if (notificationData.contains("attachment")) {
            QUrl url = QUrl(QString::fromStdString(static_cast<std::string>(notificationData["attachment"]["url"])));
            QPushButton* button = new QPushButton(this);
            button->setText("Open Attachment");
            connect(button, &QPushButton::clicked, this, [this, url]() { QDesktopServices::openUrl(url); }, Qt::QueuedConnection);
            this->ui->buttons->addWidget(button);
        }

        if (notificationData.contains("click")) {
            QUrl url = QUrl(QString::fromStdString(static_cast<std::string>(notificationData["click"])));
            QPushButton* button = new QPushButton(this);
            button->setText("Open URL");
            connect(button, &QPushButton::clicked, this, [this, url]() { QDesktopServices::openUrl(url); }, Qt::QueuedConnection);
            this->ui->buttons->addWidget(button);
        }

        if (notificationData.contains("actions")) {
            for (nlohmann::json action: notificationData["actions"]) {
                try {
                    QPushButton* button = new QPushButton(this);
                    button->setText(QString::fromStdString(static_cast<std::string>(action["label"])));
                    QUrl url = QUrl(QString::fromStdString(static_cast<std::string>(action["url"])));
                    if (action["action"] == "view") {
                        connect(button, &QPushButton::clicked, this, [this, url]() { QDesktopServices::openUrl(url); }, Qt::QueuedConnection);
                    } else if (action["action"] == "http") {
                        connect(
                            button,
                            &QPushButton::clicked,
                            this,
                            [this, button, action]() {
                                button->setEnabled(false);
                                QThread* thread = new QThread;
                                AsyncCurlRequest* request = new AsyncCurlRequest(action);
                                request->moveToThread(thread);

                                connect(thread, &QThread::started, request, &AsyncCurlRequest::run);
                                connect(request, &AsyncCurlRequest::completed, this, [this, button]() { button->setEnabled(true); }, Qt::QueuedConnection);

                                connect(request, &AsyncCurlRequest::completed, request, &AsyncCurlRequest::deleteLater);
                                connect(request, &AsyncCurlRequest::completed, thread, &QThread::quit);
                                connect(thread, &QThread::finished, thread, &QThread::deleteLater);

                                thread->start();
                            },
                            Qt::QueuedConnection
                        );
                    }
                    this->ui->buttons->addWidget(button);
                } catch (const nlohmann::json::parse_error& e) {} catch (const nlohmann::json::type_error& e) {}
            }
        }
    } catch (const nlohmann::json::parse_error& e) {} catch (const nlohmann::json::type_error& e) {}
}

NotificationListItem::~NotificationListItem() { delete ui; }

PixmapFetcher::PixmapFetcher(std::string url, QObject* parent): url(url), QObject(parent) {}

void PixmapFetcher::fetchThumbnail() {
    QPixmap image;
    image.load(FileManager::urlToTempFile(QUrl(QString::fromStdString(this->url))).toLocalFile());
    emit thumbnailFetched(image.scaled(128, 128, Qt::KeepAspectRatio, Qt::FastTransformation));
}

#include <iostream> // tmp

AsyncCurlRequest::AsyncCurlRequest(const nlohmann::json& action, QObject* parent): QObject(parent) {
    try {
        this->handle = curl_easy_init();
        curl_slist* headers = NULL;

        std::string url = static_cast<std::string>(action["url"]);

        if (action.contains("headers") && action["headers"].is_object()) {
            for (const auto& [key, value]: action["headers"].items()) {
                std::string header = key + ": " + static_cast<std::string>(value);
                headers = curl_slist_append(headers, header.c_str());
            }
        }

        if (action.contains("method") && action["method"].is_string()) {
            std::string method = static_cast<std::string>(action["method"]);
            if (method == "POST") {
                curl_easy_setopt(handle, CURLOPT_POST, 1L);
            } else if (method == "GET") {
                curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
            } else if (method == "HEAD") {
                curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
            } else {
                curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, method.c_str());
            }
        } else {
            curl_easy_setopt(handle, CURLOPT_POST, 1L);
        }

        if (action.contains("body") && action["body"].is_string()) {
            std::string body = static_cast<std::string>(action["body"]);
            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, body.c_str());
            std::cerr << "Set body to \"" << body << "\"" << std::endl;
        }

        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, ND_USERAGENT);
        curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(handle, CURLOPT_TIMEOUT, 120L);
        this->ready = true;
    } catch (const nlohmann::json::parse_error& e) {}
}

void AsyncCurlRequest::run() {
    if (this->ready) { curl_easy_perform(this->handle); }
    emit completed();
}
