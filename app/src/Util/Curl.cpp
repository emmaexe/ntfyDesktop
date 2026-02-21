#include "Curl.hpp"

#include "../DataBase/DataBase.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <cstdlib>
#include <mutex>

namespace Curl {
    void init() noexcept {
        static std::once_flag init_flag;
        std::call_once(init_flag, []() {
            CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
            if (code != CURLE_OK) {
                Logger::instance().error(std::format("Curl::init: {}", curl_easy_strerror(code)));
                exit(1);
            }
        });
    }

    std::expected<Easy*, std::string> Easy::create_heap(QObject* parent) {
        init();
        CURL* handle = curl_easy_init();
        if (!handle) { return std::unexpected(std::format("Curl::create: Failed to create libcurl handle")); }
        Easy* curl = new Easy(handle, parent);

        CURLcode code = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, curl->m_error.data());
        if (code != CURLE_OK) { return std::unexpected(std::format("Curl::create_heap: {}", curl->m_error)); }

        return curl;
    }

    std::expected<Easy*, std::string> Easy::create_with_defaults_heap(QObject* parent) {
        auto handle = Easy::create_heap(parent);

        if (handle) {
            (*handle)->set_opt(CURLOPT_USERAGENT, ND_USERAGENT);

            bool verify_tls;
            std::string ca_path;
            {
                DataBase db;
                verify_tls = db.getTlsVerificationPreference();
                ca_path = db.getCAPathPreference();
            }

            (*handle)->set_opt(CURLOPT_SSL_VERIFYPEER, verify_tls ? 1L : 0L);
            if (!ca_path.empty()) { (*handle)->set_opt(Util::Strings::endsWith(ca_path, "/") ? CURLOPT_CAPATH : CURLOPT_CAINFO, ca_path.c_str()); }

            if (Logger::instance().debug_mode) { (*handle)->set_opt(CURLOPT_VERBOSE, 1L); }

            return handle;
        } else {
            return std::unexpected(std::format("Curl::create_with_defaults_heap: {}", handle.error()));
        }
    }

    Easy::~Easy() {
        if (this->m_handle) {
            curl_easy_cleanup(this->m_handle);
            this->m_handle = nullptr;
        }
    }

    std::expected<void, std::string> Easy::set_opt(CURLoption option, const std::string& value) noexcept {
        return this->set_opt(option, value.c_str());
    }

    std::expected<void, std::string> Easy::set_opt(CURLoption option, const List& value) noexcept {
        return this->set_opt(option, value.handle());
    }

    std::expected<void, std::string> Easy::perform() noexcept {
        this->m_last_req = curl_easy_perform(this->m_handle);
        if (this->m_last_req != CURLE_OK && this->m_last_req != CURLE_ABORTED_BY_CALLBACK) { return std::unexpected(std::format("Curl::perform error: {}", this->m_error)); }
    }

    CURL* Easy::handle() const noexcept { return this->m_handle; }

    Easy::Easy(CURL* handle, QObject* parent): QObject(parent), m_handle(handle) {}

    List* List::create_heap(QObject* parent) {
        init();
        return new List(nullptr, parent);
    }

    std::expected<List*, std::string> List::create_heap(std::initializer_list<std::string> items, QObject* parent) {
        init();
        List* list = new List(nullptr, parent);

        for (const std::string& item: items) {
            auto res = list->append(item);
            if (!res) { return std::unexpected(std::format("CurlList::create_heap: {}", res.error())); }
        }

        return list;
    }

    std::expected<List*, std::string> List::create_heap(const std::vector<std::string>& items, QObject* parent) {
        init();
        List* list = new List(nullptr, parent);

        for (const std::string& item: items) {
            auto res = list->append(item);
            if (!res) { return std::unexpected(std::format("CurlList::create_heap: {}", res.error())); }
        }

        return list;
    }

    List::~List() {
        if (this->m_list) {
            curl_slist_free_all(this->m_list);
            this->m_list = nullptr;
        }
    }

    curl_slist* List::handle() const noexcept { return this->m_list; }

    std::expected<void, std::string> List::append(const std::string& item) {
        curl_slist* list = curl_slist_append(this->m_list, item.c_str());
        if (list) {
            this->m_list = list;
        } else {
            return std::unexpected("CurlList::append: Failed to append");
        }
    }

    List::List(curl_slist* list, QObject* parent) noexcept: QObject(parent), m_list(list) {}

    std::expected<Worker*, std::string> Worker::create_heap(QObject* parent) noexcept {
        auto instance = Easy::create_with_defaults_heap();
        if (instance) {
            Worker* worker = new Worker(*instance, parent);
            (*instance)->setParent(worker);

            (*instance)->set_opt(CURLOPT_WRITEDATA, worker);
            (*instance)->set_opt(
                CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                    Worker* worker = static_cast<Worker*>(userdata);

                    if (worker->m_cancelled) { return 0; }

                    QByteArray chunk(ptr, size * nmemb);
                    emit worker->write(chunk);

                    return size * nmemb;
                }
            );

            (*instance)->set_opt(CURLOPT_NOPROGRESS, 0L);
            (*instance)->set_opt(CURLOPT_XFERINFODATA, worker);
            (*instance)->set_opt(
                CURLOPT_XFERINFOFUNCTION, +[](void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) -> int {
                    Worker* worker = static_cast<Worker*>(clientp);

                    if (worker->m_cancelled) { return 1; }

                    emit worker->progress(dltotal, dlnow, ultotal, ulnow);

                    return 0;
                }
            );

            return worker;
        } else {
            return std::unexpected(std::format("CurlWorker::create_heap: {}", instance.error()));
        }
    }

    Easy& Worker::curl() { return *this->m_curl; }

    void Worker::perform() {
        this->m_cancelled = false;
        auto res = this->m_curl->perform();
        this->m_cancelled = false;
        emit finished(res);
    }

    void Worker::cancel() { this->m_cancelled = true; }

    Worker::Worker(Easy* instance, QObject* parent): QObject(parent), m_curl(instance) {}

    std::expected<Bundle*, std::string> Bundle::create_heap(QObject* parent) noexcept {
        auto worker_res = Worker::create_heap();
        if (worker_res) {
            Worker* worker = *worker_res;
            QThread* thread = new QThread;
            Bundle* bundle = new Bundle(worker, thread, parent);

            thread->setParent(bundle);
            worker->moveToThread(thread);

            QObject::connect(worker, &Worker::finished, bundle, [bundle] { bundle->m_running = false; });

            return bundle;
        } else {
            return std::unexpected(std::format("CurlBundle::create_heap: {}", worker_res.error()));
        }
    }

    Bundle::~Bundle() {
        if (this->m_running) {
            m_worker->cancel();
        }

        this->m_thread->quit();
        this->m_thread->wait();

        delete this->m_worker;
    }

    Worker& Bundle::worker() noexcept { return *this->m_worker; }

    QThread& Bundle::thread() noexcept { return *this->m_thread; }

    std::expected<void, std::string> Bundle::start() {
        if (this->m_running) { return std::unexpected("Bundle::start: The worker is already running"); }
        if (!this->m_thread->isRunning()) { this->m_thread->start(); }

        this->m_running = true;

        QMetaObject::invokeMethod(this->m_worker, &Worker::perform, Qt::QueuedConnection);
    }

    void Bundle::stop() { this->m_worker->cancel(); }

    Bundle::Bundle(Worker* worker, QThread* thread, QObject* parent): QObject(parent), m_worker(worker), m_thread(thread) {}
}
