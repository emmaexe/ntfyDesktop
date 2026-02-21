#pragma once

#include <curl/curl.h>

#include <QObject>
#include <QThread>
#include <atomic>
#include <expected>
#include <string>
#include <vector>

namespace Curl {
    /**
     * @brief This function initializes libcurl (eager loading).
     * If it is not called at the start of the program, lazy loading will be used instead.
     */
    void init() noexcept;

    template<typename T>
    concept CurlSetOptType = std::is_same_v<T, long> || std::is_same_v<T, int> || std::is_same_v<T, bool>
        || std::is_same_v<T, curl_off_t>
        || std::is_same_v<T, const char*> || std::is_same_v<T, char*>
        || std::is_pointer_v<T>;

    /**
     * @brief A RAII wrapper for a single curl easy mode instance
     */
    class Easy: public QObject {
            Q_OBJECT
        public:
            /**
             * @brief Create a new Curl instance
             */
            static std::expected<Easy*, std::string> create_heap(QObject* parent = nullptr);
            /**
             * @brief Create a new Curl instance with pre-applied connection settings and a user agent
             */
            static std::expected<Easy*, std::string> create_with_defaults_heap(QObject* parent = nullptr);

            ~Easy();

            /**
             * @brief Set an option to some value
             */
            template<CurlSetOptType T>
            std::expected<void, std::string> set_opt(CURLoption option, T value) noexcept;
            /**
             * @brief Set an option to some value
             */
            std::expected<void, std::string> set_opt(CURLoption option, const std::string& value) noexcept;
            /**
             * @brief Set an option to some value
             */
            std::expected<void, std::string> set_opt(CURLoption option, const List& value) noexcept;

            /**
             * @brief Perform a blocking curl request
             */
            std::expected<void, std::string> perform() noexcept;

            CURL* handle() const noexcept;

        private:
            Easy(CURL* handle, QObject* parent);
            CURL* m_handle = nullptr;
            CURLcode m_last_req = CURLE_OK;
            std::string m_error = std::string(CURL_ERROR_SIZE, '\0');
    };

    /**
     * @brief A RAII wrapper for curl_slist
     */
    class List: public QObject {
            Q_OBJECT
        public:
            /**
             * @brief Create a new CurlList
             */
            static List* create_heap(QObject* parent = nullptr);
            /**
             * @brief Create a new CurlList
             */
            static std::expected<List*, std::string> create_heap(std::initializer_list<std::string> items, QObject* parent = nullptr);
            /**
             * @brief Create a new CurlList
             */
            static std::expected<List*, std::string> create_heap(const std::vector<std::string>& items, QObject* parent = nullptr);

            ~List();

            curl_slist* handle() const noexcept;

            std::expected<void, std::string> append(const std::string& item);

        private:
            List(curl_slist* list, QObject* parent) noexcept;
            curl_slist* m_list = nullptr;
    };

    /**
     * @brief A Qt worker based on `Curl::Easy`
     */
    class Worker: public QObject {
            Q_OBJECT
        public:
            /**
             * @brief Create a new Curl::Worker
             */
            static std::expected<Worker*, std::string> create_heap(QObject* parent = nullptr) noexcept;

            /**
             * @brief Get this worker's Curl::Easy instance
             */
            Easy& curl();

        public slots:
            void perform();
            void cancel();

        signals:
            void write(QByteArray chunk);
            void progress(curl_off_t download_total, curl_off_t download_now, curl_off_t upload_total, curl_off_t upload_now);
            void finished(std::expected<void, std::string> res);

        private:
            Worker(Easy* instance, QObject* parent);
            Easy* m_curl;
            std::atomic_bool m_cancelled = false;
    };

    /**
     * @brief A reusable bundle holding a QThread and CurlWorker pair
     */
    class Bundle: public QObject {
            Q_OBJECT
        public:
            /**
             * @brief Create a new CurlBundle
             */
            static std::expected<Bundle*, std::string> create_heap(QObject* parent = nullptr) noexcept;

            ~Bundle();

            /**
             * @brief Get the worker stored in the bundle
             */
            Worker& worker() noexcept;
            /**
             * @brief Get the thread stored in the bundle
             */
            QThread& thread() noexcept;

        public slots:
            std::expected<void, std::string> start();
            void stop();

        private:
            Bundle(Worker* worker, QThread* thread, QObject* parent);
            Worker* m_worker;
            QThread* m_thread;
            bool m_running = false;
    };
}

#include "Curl.tpp"
