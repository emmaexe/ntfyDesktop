#pragma once

#include <string>
#include <mutex>

/**
 * @brief A singleton that can be used to log messages and errors
 */
class Logger {
    public:
        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;
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
        Logger();
        std::mutex mutex;
};
