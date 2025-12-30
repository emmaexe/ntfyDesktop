#pragma once

#include <QObject>
#include <mutex>
#include <string>

/**
 * @brief A singleton that can be used to log messages and errors
 */
class Logger: public QObject {
        Q_OBJECT
        Q_DISABLE_COPY(Logger)
    public:
        /**
         * @brief Get the Logger instance
         */
        static Logger& get();

        /**
         * @brief Use to help with debugging; Only active when the ND_DEBUG env variable is set
         */
        void debug(std::string_view message);
        /**
         * @brief Use for general logs
         */
        void log(std::string_view message);
        /**
         * @brief Use when an error occurs
         */
        void error(std::string_view message);

        const bool debugMode = false;
    private:
        Logger(QObject* parent = nullptr);
        std::mutex mutex;
};
