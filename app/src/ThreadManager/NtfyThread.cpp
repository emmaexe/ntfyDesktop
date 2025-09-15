#include "NtfyThread.hpp"

#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Curl.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <emojicpp/emoji.hpp>
#include <nlohmann/json.hpp>

#include <KLocalizedString>
#include <QApplication>
#include <QString>
#include <iostream>

NtfyThread::NtfyThread(
    std::string name, std::string protocol, std::string domain, std::string topic, AuthConfig authConfig, int lastTimestamp, bool verifyTls, std::string CAPath, std::optional<int> reconnectCount,
    std::optional<int> timeout, std::mutex* mutex, bool pollMode
):
    internalName(name),
    internalDomain(domain),
    internalTopic(topic),
    internalAuthConfig(authConfig),
    internalProtocol(protocol),
    lastTimestamp(lastTimestamp),
    verifyTls(verifyTls),
    CAPath(CAPath),
    reconnectCount(reconnectCount),
    timeout(timeout),
    mutex(mutex),
    pollMode(pollMode) {
    if (!(this->internalProtocol == "https" || this->internalProtocol == "http" || this->internalProtocol == "wss" || this->internalProtocol == "ws")) { this->internalProtocol = "https"; }
    this->url = this->internalProtocol + "://" + this->internalDomain + "/" + this->internalTopic + (Util::Strings::startsWith(this->internalProtocol, "ws") ? "/ws" : "/json");
    this->thread = std::thread(&NtfyThread::run, this);
}

NtfyThread::~NtfyThread() { this->stop(); }

void NtfyThread::run() {
    this->running = true;

    Curl curlInstance = Curl::withDefaults();
    CurlList headers;

    if (this->internalAuthConfig.type == AuthType::USERNAME_PASSWORD) {
        QByteArray base64 = QString::fromStdString(this->internalAuthConfig.username + ":" + this->internalAuthConfig.password).toUtf8().toBase64();
        headers.append("Authorization: Basic " + std::string(base64.constData(), base64.length()));
    } else if (this->internalAuthConfig.type == AuthType::TOKEN) {
        headers.append("Authorization: Bearer " + this->internalAuthConfig.token);
    }

    curlInstance.setOpt(CURLOPT_WRITEFUNCTION, &NtfyThread::writeCallback);
    curlInstance.setOpt(CURLOPT_WRITEDATA, this);
    curlInstance.setOpt(CURLOPT_XFERINFOFUNCTION, &NtfyThread::progressCallback);
    curlInstance.setOpt(CURLOPT_XFERINFODATA, this);
    curlInstance.setOpt(CURLOPT_NOPROGRESS, 0L);
    curlInstance.setOpt(CURLOPT_CONNECTTIMEOUT, 10L);
    curlInstance.setOpt(CURLOPT_HTTPHEADER, headers.handle());

    if (this->pollMode) {
        std::string currentUrl = this->url + "?poll=1&since=all";
        curlInstance.setOpt(CURLOPT_URL, currentUrl.c_str());

        CURLcode res = curl_easy_perform(curlInstance.handle());
        if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR) { this->runCount++; }

        this->running = false;
    } else {
        while (this->running) {
            if (reconnectCount.has_value() && this->runCount > reconnectCount.value()) {
                break;
            } else if (this->runCount > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(this->timeout.value()));
            }

            std::string currentUrl = this->url + "?since=" + (this->lastTimestamp > 0 ? std::to_string(this->lastTimestamp + 1) : "all");
            curlInstance.setOpt(CURLOPT_URL, currentUrl.c_str());

            this->runCount++;
            CURLcode res = curl_easy_perform(curlInstance.handle());
            if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR) { std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl; }
        }
    }

    if (reconnectCount.has_value() && this->runCount > reconnectCount.value() && !pollMode) {
        this->running = false;
        const std::string title = "Unable to connect to " + this->internalName;
        const std::string message = "Maximum retries exceeded for notification source \"" + this->internalName + "\" (" + this->url + ")";
        QMetaObject::invokeMethod(QApplication::instance(), [title, message]() { NotificationManager::errorNotification(title, message); });
    }
}

void NtfyThread::stop() {
    this->running = false;
    if (this->thread.joinable()) { this->thread.join(); }
}

void NtfyThread::waitForStop() {
    if (this->thread.joinable()) { this->thread.join(); }
}

size_t NtfyThread::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    NtfyThread* this_p = static_cast<NtfyThread*>(userdata);
    std::vector<std::string> data = Util::Strings::split(std::string(ptr, size * nmemb), "\n");

    if (!this_p->running) { return 0; }

    DataBase db;

    for (const std::string& line: data) {
        if (line.empty()) { continue; }
        try {
            nlohmann::json jsonData = nlohmann::json::parse(line);
            if (jsonData["event"] == "message") {
                this_p->lastTimestamp = jsonData["time"].get<int>();
                try {
                    NtfyNotification notification(jsonData, this_p->internalDomain, this_p->internalTopic);
                    QMetaObject::invokeMethod(QApplication::instance(), [notification]() { NotificationManager::generalNotification(notification); });
                    db.enqueueNotification(notification);
                } catch (const NtfyNotificationException& e) {
                    this_p->mutex->lock();
                    std::cerr << "Failed to parse notification: " << e.what() << std::endl;
                    this_p->mutex->unlock();
                }
            }
        } catch (const nlohmann::json::parse_error& e) {
            this_p->mutex->lock();
            std::cerr << "Malformed JSON data from notification source: " << this_p->url << '\n' << e.what() << std::endl;
            this_p->mutex->unlock();
        }
    }

    db.commitNotificationQueue();

    return size * nmemb;
}

int NtfyThread::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    NtfyThread* this_p = static_cast<NtfyThread*>(clientp);

    return this_p->running ? 0 : 1;
}

const bool NtfyThread::hasError() { return this->runCount != 0; }

const std::string& NtfyThread::name() { return this->internalName; }

const std::string& NtfyThread::protocol() { return this->internalProtocol; }

const std::string& NtfyThread::domain() { return this->internalDomain; }

const std::string& NtfyThread::topic() { return this->internalTopic; }

const AuthConfig& NtfyThread::authConfig() { return this->internalAuthConfig; }
