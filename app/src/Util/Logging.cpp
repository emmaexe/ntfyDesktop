#include "Logging.hpp"

#include "../Util/Util.hpp"

#include <QApplication>
#include <print>

Logger& Logger::instance() {
    static Logger* instance = new Logger(QApplication::instance());
    return *instance;
}

void Logger::debug(std::string_view message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    if (this->debug_mode) { std::println(stderr, "[debug] {}", message); }
}

void Logger::log(std::string_view message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    std::println(stderr, "[log] {}", message);
}

void Logger::error(std::string_view message) {
    std::lock_guard<std::mutex> lock(this->mutex);
    std::println(stderr, "[error] {}", message);
}

Logger::Logger(QObject* parent): QObject(parent), debug_mode(Util::Env::Commons::debug.has_value()) {}
