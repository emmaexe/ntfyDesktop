#include "NtfyThread.hpp"

#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <emojicpp/emoji.hpp>
#include <nlohmann/json.hpp>

#include <KLocalizedString>
#include <QApplication>
#include <QString>
#include <iostream>

const int maxRetries = 3;
const int connectionLostTimeouts[] = { 1000, 1000, 5000 };

NtfyThread::NtfyThread(std::string name, std::string domain, std::string topic, AuthConfig authConfig, bool secure, std::string lastNotificationID, std::mutex* mutex):
    internalName(name), internalDomain(domain), internalTopic(topic), internalAuthConfig(authConfig), internalSecure(secure), lastNotificationID(lastNotificationID), mutex(mutex) {
    this->url = (secure ? "https://" : "http://") + domain + "/" + topic + "/json";
    this->thread = std::thread(&NtfyThread::run, this);
}

NtfyThread::~NtfyThread() { this->stop(); }

void NtfyThread::run() {
    this->running = true;

    CURL* curlHandle = curl_easy_init();
    curl_slist* headers = NULL;

    if (this->internalAuthConfig.type == AuthType::USERNAME_PASSWORD) {
        QByteArray base64 = QString::fromStdString(this->internalAuthConfig.username + ":" + this->internalAuthConfig.password).toUtf8().toBase64();
        std::string header = "Authorization: Basic " + std::string(base64.constData(), base64.length());
        headers = curl_slist_append(headers, header.c_str());
    } else if (this->internalAuthConfig.type == AuthType::TOKEN) {
        std::string header = "Authorization: Bearer " + this->internalAuthConfig.token;
        headers = curl_slist_append(headers, header.c_str());
    }

    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &NtfyThread::writeCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curlHandle, CURLOPT_XFERINFOFUNCTION, &NtfyThread::progressCallback);
    curl_easy_setopt(curlHandle, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, ND_USERAGENT);
    curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, headers);

    while (this->running && this->internalErrorCounter <= maxRetries) {
        if (this->internalErrorCounter > 0) { std::this_thread::sleep_for(std::chrono::milliseconds(connectionLostTimeouts[this->internalErrorCounter - 1])); }

        std::string currentUrl = this->url + "?since=" + this->lastNotificationID;
        curl_easy_setopt(curlHandle, CURLOPT_URL, currentUrl.c_str());

        CURLcode res = curl_easy_perform(curlHandle);
        if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR) { std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl; }
        if (this->running) { this->internalErrorCounter++; }
    }

    curl_easy_cleanup(curlHandle);

    if (internalErrorCounter > maxRetries) {
        this->running = false;
        const std::string title = "Unable to connect to " + this->internalName;
        const std::string message = "Maximum retries exceeded for notification source \"" + this->internalName + "\" (" + this->url + ")";
        QMetaObject::invokeMethod(QApplication::instance(), [title, message]() { NotificationManager::errorNotification(title, message); });
    }
}

const std::string& NtfyThread::stop() {
    this->running = false;
    if (this->thread.joinable()) { this->thread.join(); }
    return this->lastNotificationID;
}

size_t NtfyThread::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    NtfyThread* this_p = static_cast<NtfyThread*>(userdata);
    std::vector<std::string> data = Util::split(std::string(ptr, size * nmemb), "\n");

    if (!this_p->running) { return 0; }

    for (std::string& line: data) {
        if (line.empty()) { continue; }
        try {
            nlohmann::json jsonData = nlohmann::json::parse(line);
            if (jsonData["event"] == "message") {
                std::string notificationID(jsonData["id"]);
                this_p->lastNotificationID = notificationID;

                std::string title(jsonData.contains("title") ? jsonData["title"] : jsonData["topic"]);
                std::string message(jsonData["message"]);

                if (jsonData.contains("tags")) {
                    bool seenTag = false;
                    for (nlohmann::json item: jsonData["tags"]) {
                        std::string tag = static_cast<std::string>(item);
                        std::string ptag = emojicpp::emoji::parse(":" + tag + ":");
                        if (":" + tag + ":" == ptag) {
                            if (!seenTag) {
                                seenTag = true;
                                message += " Tags: ";
                            }
                            message += tag + " ";
                        } else {
                            title = ptag + " " + title;
                        }
                    }
                }

                std::optional<NotificationPriority> priority = std::nullopt;
                if (jsonData.contains("priority")) { priority = static_cast<NotificationPriority>(jsonData["priority"].get<int>()); }

                std::optional<NotificationAttachment> attachment = std::nullopt;
                if (jsonData.contains("icon")) {
                    attachment = NotificationAttachment();
                    attachment.value().type = NotificationAttachmentType::ICON;
                    attachment.value().url = jsonData["icon"];
                }
                if (jsonData.contains("attachment")) {
                    attachment = NotificationAttachment();
                    attachment.value().type = NotificationAttachmentType::IMAGE;
                    attachment.value().name = jsonData["attachment"]["name"];
                    attachment.value().url = jsonData["attachment"]["url"];
                    attachment.value().native = jsonData["attachment"].contains("type");
                }

                std::optional<std::vector<NotificationAction>> actions = std::nullopt;
                if (jsonData.contains("click")) {
                    if (!actions.has_value()) { actions = std::vector<NotificationAction>({}); }
                    actions.value().push_back(NotificationAction(static_cast<std::string>(jsonData["click"])));
                }
                if (jsonData.contains("actions")) {
                    if (!actions.has_value()) { actions = std::vector<NotificationAction>({}); }
                    for (nlohmann::json element: jsonData["actions"]) { actions.value().push_back(NotificationAction(element)); }
                }

                QMetaObject::invokeMethod(QApplication::instance(), [title, message, priority, attachment, actions]() {
                    NotificationManager::generalNotification(title, message, priority, attachment, actions);
                });
            }
        } catch (nlohmann::json::parse_error e) {
            this_p->mutex->lock();
            std::cerr << "Malformed JSON data from notification source: " << this_p->url + "?since=" + this_p->lastNotificationID << '\n' << e.what() << std::endl;
            this_p->mutex->unlock();
        }
    }

    return size * nmemb;
}

int NtfyThread::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    NtfyThread* this_p = static_cast<NtfyThread*>(clientp);

    if (!this_p->running) { return 1; }

    return 0;
}

const std::string& NtfyThread::name() { return this->internalName; }

const std::string& NtfyThread::domain() { return this->internalDomain; }

const std::string& NtfyThread::topic() { return this->internalTopic; }

const AuthConfig& NtfyThread::authConfig() { return this->internalAuthConfig; }

const bool NtfyThread::secure() { return this->internalSecure; }
