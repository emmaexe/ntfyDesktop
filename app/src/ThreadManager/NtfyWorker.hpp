#pragma once

#include "../DataBase/DataBase.hpp"
#include "../Util/Curl.hpp"

#include <QObject>
#include <QThread>
#include <atomic>
#include <optional>
#include <string>
#include <vector>

namespace NtfyWorker {
    struct ConnectionOptions {
        std::string workerName, protocol, domain, topic;
        AuthConfig authConfig;
        std::optional<int> lastTimestamp = std::nullopt, reconnectCount = std::nullopt, timeout = std::nullopt;
        std::optional<std::string> CAPath = std::nullopt;
        bool verifyTls = true;
    };

    struct Error {
        CURLcode curlCode;
        std::string what;
    };

    struct ExitData {
        ConnectionOptions options;
        std::vector<Error> errors;
        bool fatalError = false;
    };

    class BaseWorker: public QObject {
            Q_OBJECT
        public:
            BaseWorker(const ConnectionOptions& options, QObject* parent = nullptr);
        public slots:
            void run();
            void stop();
        signals:
            void finished(ExitData exit);
        protected:
            virtual void makeRequest(Curl& curlInstance, const std::string& url, ExitData& exitData) = 0;
            virtual size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) = 0;
            virtual int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) = 0;
            static size_t writeCallbackTrampoline(char* ptr, size_t size, size_t nmemb, void* userdata);
            static int progressCallbackTrampoline(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
            ConnectionOptions options;
            std::atomic<bool> running = false;
    };

    struct Bundle {
        std::unique_ptr<BaseWorker> worker;
        std::unique_ptr<QThread> thread;
    };

    class NtfyWorker: public BaseWorker {
            Q_OBJECT
        public:
            NtfyWorker(const ConnectionOptions& options, QObject* parent = nullptr);
        private:
            void makeRequest(Curl& curlInstance, const std::string& url, ExitData& exitData) override;
            size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) override;
            int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) override;
    };

    class PollWorker: public BaseWorker {
            Q_OBJECT
        public:
            PollWorker(const ConnectionOptions& options, QObject* parent = nullptr);
        private:
            void makeRequest(Curl& curlInstance, const std::string& url, ExitData& exitData) override;
            size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) override;
            int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) override;
    };
}

Q_DECLARE_METATYPE(NtfyWorker::ConnectionOptions);
Q_DECLARE_METATYPE(NtfyWorker::Error);
Q_DECLARE_METATYPE(NtfyWorker::ExitData);
