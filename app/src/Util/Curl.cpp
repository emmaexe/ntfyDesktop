#include "Curl.hpp"

#include "../DataBase/DataBase.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <stdexcept>

Curl::Curl() {
    this->internalHandle = curl_easy_init();
    if (!this->internalHandle) { throw std::runtime_error("Failed to initialize CURL internalHandle"); }
}

Curl::~Curl() {
    if (this->internalHandle) {
        curl_easy_cleanup(this->internalHandle);
        this->internalHandle = nullptr;
    }
}

Curl::Curl(Curl&& other) noexcept: internalHandle(other.internalHandle) { other.internalHandle = nullptr; }

Curl& Curl::operator=(Curl&& other) noexcept {
    if (this != &other) {
        if (this->internalHandle) {
            curl_easy_cleanup(this->internalHandle);
            this->internalHandle = nullptr;
        }
        this->internalHandle = other.internalHandle;
        other.internalHandle = nullptr;
    }
    return *this;
}

CURL* Curl::handle() const noexcept { return this->internalHandle; }

Curl Curl::withDefaults() {
    Curl handle;
    handle.setOpt(CURLOPT_USERAGENT, ND_USERAGENT);

    bool verifyTls;
    std::string CAPath;
    {
        DataBase db;
        verifyTls = db.getTlsVerificationPreference();
        CAPath = db.getCAPathPreference();
    }

    handle.setOpt(CURLOPT_SSL_VERIFYPEER, verifyTls ? 1L : 0L);
    if (!CAPath.empty()) { handle.setOpt(Util::Strings::endsWith(CAPath, "/") ? CURLOPT_CAPATH : CURLOPT_CAINFO, CAPath.c_str()); }

    if (Logger::get().debugModeActive()) { handle.setOpt(CURLOPT_VERBOSE, 1L); }

    return handle;
}

CurlList::CurlList() noexcept {}

CurlList::CurlList(std::initializer_list<std::string> items) noexcept {
    for (const std::string& item: items) { this->append(item); }
}

CurlList::CurlList(const std::vector<std::string>& items) noexcept {
    for (const std::string& item: items) { this->append(item); }
}

CurlList::~CurlList() {
    if (this->internalList) {
        curl_slist_free_all(this->internalList);
        this->internalList = nullptr;
    }
}

CurlList::CurlList(CurlList&& other) noexcept: internalList(other.internalList) { other.internalList = nullptr; }

CurlList& CurlList::operator=(CurlList&& other) noexcept {
    if (this != &other) {
        if (this->internalList) {
            curl_slist_free_all(this->internalList);
            this->internalList = nullptr;
        }
        this->internalList = other.internalList;
        other.internalList = nullptr;
    }
    return *this;
}

curl_slist* CurlList::handle() const noexcept { return this->internalList; }

void CurlList::append(const std::string& item) {
    curl_slist* newInternalList = curl_slist_append(this->internalList, item.c_str());
    if (newInternalList) {
        this->internalList = newInternalList;
    } else {
        throw std::runtime_error("Failed to append to CurlList");
    }
}
