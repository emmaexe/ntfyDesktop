#include "ThreadManager.hpp"

#include "../MainWindow/MainWindow.hpp"
#include "../NotificationManager/NotificationManager.hpp"

#include <iostream>
#include <QApplication>

ThreadManager::ThreadManager(QObject* parent): QObject(parent) { this->restartConfig(); }

ThreadManager::~ThreadManager() {}

void ThreadManager::stopAll() {
    for (std::unique_ptr<NtfyThread>& thread_p: this->threads) { thread_p->stop(); }
    this->threads.clear();
}

void ThreadManager::restartConfig() {
    this->stopAll();
    for (nlohmann::json& source: Config::data()["sources"]) {
        try {
            std::string url = "https://" + std::string(source["server"]) + "/" + std::string(source["topic"]) + "/json";
            this->threads.push_back(std::make_unique<NtfyThread>(url, &this->mutex));
        } catch (nlohmann::json::out_of_range e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
    }
}
