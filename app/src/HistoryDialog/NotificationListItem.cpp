#include "NotificationListItem.hpp"

#include "../DataBase/DataBase.hpp"
#include "../NotificationManager/NtfyNotification.hpp"
#include "../Util/FileManager.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"
#include "ui_NotificationListItem.h"

#include <QDesktopServices>
#include <QListWidgetItem>
#include <QPushButton>
#include <QThread>
#include <format>

using Util::Colors::ColorMode;

NotificationListItem::NotificationListItem(
    std::string_view id, std::string_view server, std::string_view topic, const int& timestamp, std::string_view title, std::string_view message, std::string_view rawData, QWidget* parent
):
    QWidget(parent),
    ui(new Ui::NotificationListItem),
    internalId(id),
    internalServer(server),
    internalTopic(topic),
    internalTimestamp(timestamp),
    internalTitle(std::format("<b>{}</b>", title)),
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
                                        Util::Colors::setButtonColor(*button, (success ? ColorMode::Success : ColorMode::Failure));
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

        this->internalValid = true;
    } catch (const nlohmann::json::parse_error& ignored) {
    } catch (const nlohmann::json::type_error& ignored) {}
}

NotificationListItem::~NotificationListItem() { delete ui; }

const bool& NotificationListItem::valid() { return this->internalValid; }

const std::string& NotificationListItem::id() { return this->internalId; }
const std::string& NotificationListItem::server() { return this->internalServer; }
const std::string& NotificationListItem::topic() { return this->internalTopic; }
const int& NotificationListItem::timestamp() { return this->internalTimestamp; }
const std::string& NotificationListItem::title() { return this->internalTitle; }
const std::string& NotificationListItem::message() { return this->internalMessage; }

PixmapFetcher::PixmapFetcher(std::string url, QObject* parent): url(url), QObject(parent) {}

void PixmapFetcher::fetchThumbnail() {
    QPixmap image;
    image.load(FileManager::urlToTempFile(QUrl(QString::fromStdString(this->url))).toLocalFile());
    emit thumbnailFetched(image.scaled(128, 128, Qt::KeepAspectRatio, Qt::FastTransformation));
}

AsyncCurlRequest::AsyncCurlRequest(const nlohmann::json& action, QObject* parent): QObject(parent) {
    try {
        this->url = static_cast<std::string>(action["url"]);

        if (action.contains("headers") && action["headers"].is_object()) {
            for (const auto& [key, value]: action["headers"].items()) {
                rawHeaders.push_back(key + ": " + static_cast<std::string>(value));
                headers.append(this->rawHeaders.at(this->rawHeaders.size() - 1).c_str());
            }
        }

        if (action.contains("method") && action["method"].is_string()) {
            this->method = static_cast<std::string>(action["method"]);
            if (this->method == "POST") {
                this->curlInstance.setOpt(CURLOPT_POST, 1L);
            } else if (this->method == "GET") {
                this->curlInstance.setOpt(CURLOPT_HTTPGET, 1L);
            } else if (this->method == "HEAD") {
                this->curlInstance.setOpt(CURLOPT_NOBODY, 1L);
            } else {
                this->curlInstance.setOpt(CURLOPT_CUSTOMREQUEST, this->method.c_str());
            }
        } else {
            this->curlInstance.setOpt(CURLOPT_POST, 1L);
        }

        if (action.contains("body") && action["body"].is_string()) {
            this->body = static_cast<std::string>(action["body"]);
            this->curlInstance.setOpt(CURLOPT_COPYPOSTFIELDS, this->body.c_str());
        }

        this->curlInstance.setOpt(CURLOPT_WRITEFUNCTION, &AsyncCurlRequest::writeCallback);
        this->curlInstance.setOpt(CURLOPT_WRITEDATA, this);
        this->curlInstance.setOpt(CURLOPT_URL, this->url.c_str());
        this->curlInstance.setOpt(CURLOPT_HTTPHEADER, headers.handle());
        this->curlInstance.setOpt(CURLOPT_TIMEOUT, 120L);

        this->ready = true;
    } catch (const nlohmann::json::parse_error& e) {}
}

void AsyncCurlRequest::run() {
    if (this->ready) {
        CURLcode res = curl_easy_perform(this->curlInstance.handle());
        if (res == CURLE_OK) {
            long httpCode = 0;
            curl_easy_getinfo(this->curlInstance.handle(), CURLINFO_RESPONSE_CODE, &httpCode);
            if (200 <= httpCode && httpCode <= 299) {
                emit completed(true);
                return;
            }
        }
    }
    emit completed(false);
}

size_t AsyncCurlRequest::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) { return size * nmemb; }
