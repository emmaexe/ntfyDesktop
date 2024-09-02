#include "NtfyThread.hpp"

#include <nlohmann/json.hpp>
#include <iostream>

#include <KNotification>
#include <KLocalizedString>
#include <QApplication>

#include <emojicpp/emoji.hpp>

#include "../NotificationManager/NotificationManager.hpp"

NtfyThread::NtfyThread(std::string url, std::mutex* mutex): url(url), mutex(mutex) {
    this->thread = std::thread(&NtfyThread::run, this);
}

NtfyThread::~NtfyThread() {
    this->stop();
}

void NtfyThread::run() {
    CURL* curlHandle = curl_easy_init();

    curl_easy_setopt(curlHandle, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &NtfyThread::writeCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curlHandle, CURLOPT_XFERINFOFUNCTION, &NtfyThread::progressCallback);
    curl_easy_setopt(curlHandle, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1L);

    this->running = true;

    bool firstRun = true;

    while(this->running) {
        if (!firstRun) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CONNECTION_LOST_TIMEOUT));
        }
        firstRun = false;
        CURLcode res = curl_easy_perform(curlHandle);
        if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR) { std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl; }
    }

    curl_easy_cleanup(curlHandle);
}

void NtfyThread::stop() {
    this->running = false;
    if (this->thread.joinable()) {
        this->thread.join();
    }
}

size_t NtfyThread::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    NtfyThread* this_p = static_cast<NtfyThread*>(userdata);
    std::string data(ptr, size*nmemb);

    if (!this_p->running) {
        return 0;
    }

    try {
        nlohmann::json jsonData = nlohmann::json::parse(data);
        if (jsonData["event"] == "message") {
            std::string title(jsonData.contains("title") ? jsonData["title"] : jsonData["topic"]);
            std::string message(jsonData["message"]);
            std::optional<NotificationPriority> priority = std::nullopt;
            if (jsonData.contains("priority")) {
                priority = static_cast<NotificationPriority>(jsonData["priority"].get<int>());
            }
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
                if (!actions.has_value()) {
                    actions = std::vector<NotificationAction>({});
                }
                actions.value().push_back(NotificationAction(static_cast<std::string>(jsonData["click"])));
            }
            if (jsonData.contains("actions")) {
                if (!actions.has_value()) {
                    actions = std::vector<NotificationAction>({});
                }
                for (nlohmann::json element: jsonData["actions"]) {
                    actions.value().push_back(NotificationAction(element));
                }
            }
            QMetaObject::invokeMethod(qApp, NotificationManager::generalNotification, title, message, priority, attachment, actions);
        }
    } catch(nlohmann::json::parse_error e) {
        this_p->mutex->lock();
        std::cerr << "Malformed JSON data from notification source: " << this_p->url << '\n' << e.what() << std::endl;
        this_p->mutex->unlock();
    }

    return size*nmemb;
}

int NtfyThread::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    NtfyThread* this_p = static_cast<NtfyThread*>(clientp);

    if (!this_p->running) {
        return 1;
    }

    return 0;
}
