#pragma once

#include <curl/curl.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

// How long to wait for after the connection fails before retrying.
#define CONNECTION_LOST_TIMEOUT 1000

class NtfyThread {
    public:
        NtfyThread(std::string domain, std::string topic, bool secure, std::string lastNotificationID, std::mutex* mutex);
        ~NtfyThread();
        void run();
        const std::string& stop();
        const std::string& domain();
        const std::string& topic();
    private:
        std::thread thread;
        std::mutex* mutex;
        std::atomic<bool> running;
        std::string internalDomain, internalTopic, url, lastNotificationID;
        static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
};
