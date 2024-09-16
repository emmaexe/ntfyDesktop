#include "ThreadManager.hpp"

#include "../MainWindow/MainWindow.hpp"
#include "../NotificationManager/NotificationManager.hpp"
#include "../NotificationStore/NotificationStore.hpp"

#include <QApplication>
#include <iostream>

ThreadManager::ThreadManager(QObject* parent): QObject(parent) { this->restartConfig(); }

ThreadManager::~ThreadManager() {}

void ThreadManager::stopAll() {
    for (std::unique_ptr<NtfyThread>& thread_p: this->threads) {
        std::string lastNotificationID = thread_p->stop();
        NotificationStore::update(thread_p->domain(), thread_p->topic(), lastNotificationID);
    }
    this->threads.clear();
}

void ThreadManager::restartConfig() {
    this->stopAll();
    for (nlohmann::json& source: Config::data()["sources"]) {
        try {
            std::string lastNotificationID = "all", server = std::string(source["server"]), topic = std::string(source["topic"]);
            if (NotificationStore::exists(server, topic)) { lastNotificationID = NotificationStore::get(server, topic).value(); }
            this->threads.push_back(std::make_unique<NtfyThread>(server, topic, true, lastNotificationID, &this->mutex));
        } catch (nlohmann::json::out_of_range e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
    }
}
