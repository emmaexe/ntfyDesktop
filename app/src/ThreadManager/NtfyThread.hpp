#pragma once

#include<atomic>
#include<thread>
#include<mutex>
#include<functional>
#include<string>
#include<curl/curl.h>

// How long to wait for after the connection fails before retrying.
#define CONNECTION_LOST_TIMEOUT 1000

class NtfyThread {
    public:
        NtfyThread(std::string url, std::mutex* mutex);
        ~NtfyThread();
        void run();
        void stop();
    private:
        std::thread thread;
        std::mutex* mutex;
        std::atomic<bool> running;
        std::string url;
        static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
};
