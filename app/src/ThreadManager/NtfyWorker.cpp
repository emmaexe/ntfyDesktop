#include "NtfyWorker.hpp"

#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Util.hpp"

#include <emojicpp/emoji.hpp>
#include <nlohmann/json.hpp>

#include <KLocalizedString>
#include <QApplication>
#include <QString>
#include <QThread>

namespace NtfyWorker {
    BaseWorker::BaseWorker(const ConnectionOptions& options, QObject* parent): QObject(parent), options(options) {}

    void BaseWorker::run() {
        this->running = true;
        Logger& logger = Logger::get();
        ExitData exitData{ this->options };

        Curl curlInstance = Curl::withDefaults();
        CurlList headers;

        if (this->options.authConfig.type == AuthType::USERNAME_PASSWORD) {
            QByteArray base64 = QString::fromStdString(this->options.authConfig.username + ":" + this->options.authConfig.password).toUtf8().toBase64();
            headers.append("Authorization: Basic " + std::string(base64.constData(), base64.length()));
        } else if (this->options.authConfig.type == AuthType::TOKEN) {
            headers.append("Authorization: Bearer " + this->options.authConfig.token);
        }

        curlInstance.setOpt(CURLOPT_WRITEFUNCTION, &BaseWorker::writeCallbackTrampoline);
        curlInstance.setOpt(CURLOPT_WRITEDATA, this);
        curlInstance.setOpt(CURLOPT_XFERINFOFUNCTION, &BaseWorker::progressCallbackTrampoline);
        curlInstance.setOpt(CURLOPT_XFERINFODATA, this);
        curlInstance.setOpt(CURLOPT_NOPROGRESS, 0L);
        curlInstance.setOpt(CURLOPT_CONNECTTIMEOUT, 10L);
        curlInstance.setOpt(CURLOPT_HTTPHEADER, headers.handle());

        std::string url = std::format(
            "{}://{}/{}/{}",
            this->options.protocol == "https" || this->options.protocol == "http" || this->options.protocol == "wss" || this->options.protocol == "ws" ? this->options.protocol : "https",
            this->options.domain,
            this->options.topic,
            Util::Strings::startsWith(this->options.protocol, "ws") ? "ws" : "json"
        );

        this->makeRequest(curlInstance, url, exitData);

        this->running = false;
        emit finished(exitData);
    }

    void BaseWorker::stop() { this->running = false; }

    size_t BaseWorker::writeCallbackTrampoline(char* ptr, size_t size, size_t nmemb, void* userdata) {
        BaseWorker* this_p = static_cast<BaseWorker*>(userdata);
        return this_p->writeCallback(ptr, size, nmemb, userdata);
    }

    int BaseWorker::progressCallbackTrampoline(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        BaseWorker* this_p = static_cast<BaseWorker*>(clientp);
        return this_p->progressCallback(clientp, dltotal, dlnow, ultotal, ulnow);
    }

    NtfyWorker::NtfyWorker(const ConnectionOptions& options, QObject* parent): BaseWorker(options, parent) {}

    void NtfyWorker::makeRequest(Curl& curlInstance, const std::string& url, ExitData& exitData) {
        Logger& logger = Logger::get();
        int runCount = 0;
        while (this->running) {
            if (this->options.reconnectCount.has_value() && runCount > this->options.reconnectCount.value()) {
                break;
            } else if (runCount > 0 && this->options.timeout.has_value()) {
                QThread::sleep(std::chrono::milliseconds(this->options.timeout.value()));
            }

            std::string currentUrl = std::format("{}?since={}", url, (this->options.lastTimestamp.has_value() ? std::to_string(this->options.lastTimestamp.value() + 1) : "all"));
            curlInstance.setOpt(CURLOPT_URL, currentUrl.c_str());

            runCount++;
            CURLcode res = curl_easy_perform(curlInstance.handle());
            if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR) {
                exitData.fatalError = true;
                exitData.errors.push_back(Error{ res, std::string(curl_easy_strerror(res)) });
                logger.error("curl error: " + exitData.errors.back().what);
            }
        }
    }

    size_t NtfyWorker::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        std::vector<std::string> data = Util::Strings::split(std::string(ptr, size * nmemb), "\n");

        if (!this->running) { return 0; }

        Logger& logger = Logger::get();
        DataBase db;

        for (const std::string& line: data) {
            if (line.empty()) { continue; }
            try {
                nlohmann::json jsonData = nlohmann::json::parse(line);
                if (jsonData["event"] == "message") {
                    this->options.lastTimestamp = std::make_optional(jsonData["time"].get<int>());
                    try {
                        NtfyNotification notification(jsonData, this->options.domain, this->options.topic);
                        QMetaObject::invokeMethod(QApplication::instance(), [notification]() { NotificationManager::generalNotification(notification); });
                        db.enqueueNotification(notification);
                    } catch (const NtfyNotificationException& e) { logger.error("Failed to parse notification: " + std::string(e.what())); }
                }
            } catch (const nlohmann::json::parse_error& e) { logger.error("Malformed JSON data from notification source " + this->options.workerName + ": " + e.what()); }
        }

        db.commitNotificationQueue();

        return size * nmemb;
    }

    int NtfyWorker::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) { return this->running ? 0 : 1; }

    PollWorker::PollWorker(const ConnectionOptions& options, QObject* parent): BaseWorker(options, parent) {}

    void PollWorker::makeRequest(Curl& curlInstance, const std::string& url, ExitData& exitData) {
        Logger& logger = Logger::get();

        std::string currentUrl = url + "?poll=1&since=all";
        curlInstance.setOpt(CURLOPT_URL, currentUrl.c_str());

        CURLcode res = curl_easy_perform(curlInstance.handle());
        if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR) {
            exitData.fatalError = true;
            exitData.errors.push_back(Error{ res, std::string(curl_easy_strerror(res)) });
            logger.error("curl error: " + exitData.errors.back().what);
        }
    }

    size_t PollWorker::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        std::vector<std::string> data = Util::Strings::split(std::string(ptr, size * nmemb), "\n");

        if (!this->running) { return 0; }

        Logger& logger = Logger::get();
        DataBase db;

        for (const std::string& line: data) {
            if (line.empty()) { continue; }
            try {
                nlohmann::json jsonData = nlohmann::json::parse(line);
                if (jsonData["event"] == "message") {
                    this->options.lastTimestamp = std::make_optional(jsonData["time"].get<int>());
                    try {
                        NtfyNotification notification(jsonData, this->options.domain, this->options.topic);
                        QMetaObject::invokeMethod(QApplication::instance(), [notification]() { NotificationManager::generalNotification(notification); });
                        db.enqueueNotification(notification);
                    } catch (const NtfyNotificationException& e) { logger.error("Failed to parse notification: " + std::string(e.what())); }
                }
            } catch (const nlohmann::json::parse_error& e) { logger.error("Malformed JSON data from notification source " + this->options.workerName + ": " + e.what()); }
        }

        db.commitNotificationQueue();

        return size * nmemb;
    }

    int PollWorker::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) { return this->running ? 0 : 1; }
}
