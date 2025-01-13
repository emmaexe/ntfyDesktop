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
                                connect(
                                    request,
                                    &AsyncCurlRequest::completed,
                                    this,
                                    [this, &button](bool success) {
                                        QPalette palette = button->palette();
                                        palette.setColor(QPalette::Button, (success ? Util::Colors::buttonColorSuccess() : Util::Colors::buttonColorFailure()));
                                        palette.setColor(QPalette::ButtonText, (success ? Util::Colors::buttonTextColorSuccess() : Util::Colors::buttonTextColorFailure()));
                                        button->setPalette(palette);
                                        button->setEnabled(true);
                                    },
                                    Qt::QueuedConnection
                                );

                                connect(request, &AsyncCurlRequest::completed, request, &AsyncCurlRequest::deleteLater);
                                connect(request, &AsyncCurlRequest::completed, thread, &QThread::quit);
                                connect(thread, &QThread::finished, thread, &QThread::deleteLater);

                                thread->start();
                            },
                            Qt::QueuedConnection
                        );
                    }
                    this->ui->buttons->addWidget(button);
                } catch (const nlohmann::json::parse_error& ignored) {
                } catch (const nlohmann::json::type_error& ignored) {}
            }
        }
    } catch (const nlohmann::json::parse_error& ignored) {
    } catch (const nlohmann::json::type_error& ignored) {}
}

NotificationListItem::~NotificationListItem() { delete ui; }

PixmapFetcher::PixmapFetcher(std::string url, QObject* parent): url(url), QObject(parent) {}

void PixmapFetcher::fetchThumbnail() {
    QPixmap image;
    image.load(FileManager::urlToTempFile(QUrl(QString::fromStdString(this->url))).toLocalFile());
    emit thumbnailFetched(image.scaled(128, 128, Qt::KeepAspectRatio, Qt::FastTransformation));
}

AsyncCurlRequest::AsyncCurlRequest(const nlohmann::json& action, QObject* parent): QObject(parent) {
    try {
        this->handle = curl_easy_init();
        this->headers = NULL;

        this->url = static_cast<std::string>(action["url"]);

        if (action.contains("headers") && action["headers"].is_object()) {
            for (const auto& [key, value]: action["headers"].items()) {
                rawHeaders.push_back(key + ": " + static_cast<std::string>(value));
                headers = curl_slist_append(headers, this->rawHeaders.at(this->rawHeaders.size() - 1).c_str());
            }
        }

        if (action.contains("method") && action["method"].is_string()) {
            this->method = static_cast<std::string>(action["method"]);
            if (this->method == "POST") {
                curl_easy_setopt(this->handle, CURLOPT_POST, 1L);
            } else if (this->method == "GET") {
                curl_easy_setopt(this->handle, CURLOPT_HTTPGET, 1L);
            } else if (this->method == "HEAD") {
                curl_easy_setopt(this->handle, CURLOPT_NOBODY, 1L);
            } else {
                curl_easy_setopt(this->handle, CURLOPT_CUSTOMREQUEST, this->method.c_str());
            }
        } else {
            curl_easy_setopt(this->handle, CURLOPT_POST, 1L);
        }

        if (action.contains("body") && action["body"].is_string()) {
            this->body = static_cast<std::string>(action["body"]);
            curl_easy_setopt(this->handle, CURLOPT_POSTFIELDS, this->body.c_str());
        }

        curl_easy_setopt(this->handle, CURLOPT_WRITEFUNCTION, &AsyncCurlRequest::writeCallback);
        curl_easy_setopt(this->handle, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(this->handle, CURLOPT_URL, this->url.c_str());
        curl_easy_setopt(this->handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(this->handle, CURLOPT_USERAGENT, ND_USERAGENT);
        curl_easy_setopt(this->handle, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(this->handle, CURLOPT_TIMEOUT, 120L);
        this->ready = true;
    } catch (const nlohmann::json::parse_error& e) {}
}

void AsyncCurlRequest::run() {
    if (this->ready) {
        CURLcode res = curl_easy_perform(this->handle);
        if (res == CURLE_OK) {
            long httpCode = 0;
            curl_easy_getinfo(this->handle, CURLINFO_RESPONSE_CODE, &httpCode);
            if (200 <= httpCode && httpCode <= 299) {
                emit completed(true);
                return;
            }
        }
    }
    emit completed(false);
}

size_t AsyncCurlRequest::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) { return size * nmemb; }
