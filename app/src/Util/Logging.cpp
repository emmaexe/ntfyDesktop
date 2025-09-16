#include "Logging.hpp"

#include "../Util/Util.hpp"

#include <iostream>
#include <stdexcept>

Logger& Logger::get() {
    static Logger logger;
    return logger;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    if (this->debugMode) {
        std::cerr << "[log] " << message << std::endl;
        if (this->logFile.has_value()) {
            this->logFile.value() << "[log] " << message << std::endl;
        }
    }

}

void Logger::error(const std::string& message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    std::cerr << "[error] " << message << std::endl;
    if (this->logFile.has_value()) {
        this->logFile.value() << "[error] " << message << std::endl;
    }
}

bool Logger::debugModeActive() const { return this->debugMode; }

Logger::Logger() {
    this->debugMode = Util::getEnv("ND_DEBUG").has_value();
    std::optional<std::string> logFileEnv = Util::getEnv("ND_LOGFILE");
    if (logFileEnv.has_value()) {
        this->logFile = std::make_optional<std::ofstream>(logFileEnv.value(), std::ios::app);
        if (!this->logFile.value().is_open()) {
            this->debugMode = false;
            this->logFile = std::nullopt;
            throw std::runtime_error("Failed to open log file.");
        }
    }
}
