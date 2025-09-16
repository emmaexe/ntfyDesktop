#include "Logging.hpp"

#include "../Util/Util.hpp"

#include <iostream>
#include <stdexcept>

Logger& Logger::get() {
    static Logger logger;
    static bool init = false;

    if (!init) {
        init = true;
        logger.debugMode = Util::getEnv("ND_DEBUG").has_value();
        std::optional<std::string> logFileEnv = Util::getEnv("ND_LOGFILE");
        if (logFileEnv.has_value()) {
            logger.logFile = std::make_optional<std::ofstream>(logFileEnv.value(), std::ios::app);
            if (!logger.logFile.value().is_open()) {
                init = false;
                logger.debugMode = false;
                logger.logFile = std::nullopt;
                throw std::runtime_error("Failed to open log file.");
            }
        }
    }

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

Logger::Logger() {}
