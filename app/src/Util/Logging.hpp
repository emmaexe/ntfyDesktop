#pragma once

#include <fstream>
#include <mutex>
#include <optional>
#include <string>

class Logger {
    public:
        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;
        static Logger& get();

        void debug(std::string_view message);
        void log(std::string_view message);
        void error(std::string_view message);

        const bool debugMode = false;
    private:
        Logger();
        std::mutex mutex;
};
