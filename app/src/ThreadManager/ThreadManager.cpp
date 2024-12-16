#include "ThreadManager.hpp"

#include "../DataBase/DataBase.hpp"
#include "../MainWindow/MainWindow.hpp"
#include "../NotificationManager/NotificationManager.hpp"
#include "../NotificationStore/NotificationStore.hpp"
#include "../Util/Util.hpp"

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
    DataBase db;
    for (nlohmann::json& source: Config::data()["sources"]) {
        try {
            std::string lastNotificationID = "all", name = std::string(source["name"]), domain = std::string(source["domain"]), topic = std::string(source["topic"]);
            AuthConfig authConfig = db.getAuth(Util::topicHash(domain, topic));
            bool secure = source["secure"];
            if (NotificationStore::exists(domain, topic)) { lastNotificationID = NotificationStore::get(domain, topic).value(); }
            this->threads.push_back(std::make_unique<NtfyThread>(name, domain, topic, authConfig, secure, lastNotificationID, &this->mutex));
        } catch (nlohmann::json::out_of_range e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
    }
}
