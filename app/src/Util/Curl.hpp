#pragma once

#include <curl/curl.h>

#include <string>
#include <vector>

/**
 * @brief A RAII wrapper for curl easy mode
 */
class Curl {
    public:
        Curl();
        ~Curl();

        Curl(Curl&& other) noexcept;
        Curl& operator=(Curl&& other) noexcept;

        Curl(const Curl& other) = delete;
        Curl& operator=(const Curl& other) = delete;

        template<typename T> void setOpt(CURLoption option, T value);

        CURL* handle() const noexcept;

        /**
         * @brief Recieve a curl instance with pre-applied global connection settings and a user agent
         */
        static Curl withDefaults();
    private:
        CURL* internalHandle = nullptr;
};

/**
 * @brief A RAII wrapper for curl_slist
 */
class CurlList {
    public:
        CurlList() noexcept;
        CurlList(std::initializer_list<std::string> items) noexcept;
        CurlList(const std::vector<std::string>& items) noexcept;
        ~CurlList();

        CurlList(CurlList&& other) noexcept;
        CurlList& operator=(CurlList&& other) noexcept;

        CurlList(const CurlList& other) = delete;
        CurlList& operator=(const CurlList& other) = delete;

        curl_slist* handle() const noexcept;

        void append(const std::string& item);
    private:
        curl_slist* internalList = nullptr;
};

#include "Curl.tpp"
