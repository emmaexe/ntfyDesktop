#pragma once
#include "Curl.hpp"

namespace Curl {
    template<CurlSetOptType T>
    inline std::expected<void, std::string> Easy::set_opt(CURLoption option, T value) noexcept {
        CURLcode res = curl_easy_setopt(this->m_handle, option, value);
        if (res != CURLE_OK) { return std::unexpected(std::format("Failed to set CURL option: {}", curl_easy_strerror(res))); }
    }
}
