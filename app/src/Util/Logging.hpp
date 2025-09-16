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

        void log(const std::string& message);
        void error(const std::string& message);

        bool debugModeActive() const;
    private:
        Logger();
        std::mutex mutex;
        bool debugMode = false;
        std::optional<std::ofstream> logFile = std::nullopt;
};
