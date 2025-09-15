#pragma once
#include "Curl.hpp"

#include <stdexcept>

template<typename T> inline void Curl::setOpt(CURLoption option, T value) {
    CURLcode res = curl_easy_setopt(this->internalHandle, option, value);
    if (res != CURLE_OK) { throw std::runtime_error("Failed to set CURL option: " + std::string(curl_easy_strerror(res))); }
}
