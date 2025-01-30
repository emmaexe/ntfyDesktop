#pragma once

#include "../DataBase/DataBase.hpp"

#include <curl/curl.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

class NtfyThread {
    public:
        NtfyThread(std::string name, std::string protocol, std::string domain, std::string topic, AuthConfig authConfig, int lastTimestamp, std::mutex* mutex);
        ~NtfyThread();
        void run();
        void stop();
        const std::string& name();
        const std::string& protocol();
        const std::string& domain();
        const std::string& topic();
        const AuthConfig& authConfig();
    private:
        std::thread thread;
        std::mutex* mutex;
        std::atomic<bool> running;
        std::string internalName, internalProtocol, internalDomain, internalTopic, url;
        int lastTimestamp;
        AuthConfig internalAuthConfig;
        int internalErrorCounter = 0;
        static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
};
