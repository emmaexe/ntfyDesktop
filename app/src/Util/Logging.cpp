#include "Logging.hpp"

#include "../Util/Util.hpp"

#include <print>

Logger& Logger::get() {
    static Logger logger;
    return logger;
}

void Logger::debug(std::string_view message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    if (this->debugMode) {
        std::println(stderr, "[debug] {}", message);
    }
}

void Logger::log(std::string_view message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    std::println(stderr, "[log] {}", message);
}

void Logger::error(std::string_view message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    std::println(stderr, "[error] {}", message);
}

Logger::Logger(): debugMode(Util::Env::Commons::debug.has_value()) {}
