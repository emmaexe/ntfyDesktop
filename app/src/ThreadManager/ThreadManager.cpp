#include "ThreadManager.hpp"

#include "../DataBase/DataBase.hpp"
#include "../MainWindow/MainWindow.hpp"
#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Util.hpp"

#include <QApplication>
#include <iostream>

ThreadManager::ThreadManager(QObject* parent): QObject(parent) { this->restartConfig(); }

ThreadManager::~ThreadManager() {}

void ThreadManager::stopAll() {
    for (std::unique_ptr<NtfyThread>& thread_p: this->threads) { thread_p->stop(); }
    this->threads.clear();
}

void ThreadManager::restartConfig() {
    this->stopAll();
    DataBase db;
    for (nlohmann::json& source: Config::data()["sources"]) {
        try {
            int lastTimestamp = -1;
            std::string name = source["name"].get<std::string>(), protocol = source["protocol"].get<std::string>(), domain = source["domain"].get<std::string>(), topic = source["topic"].get<std::string>(), topicHash = Util::topicHash(domain, topic);
            AuthConfig authConfig = db.getAuth(topicHash);
            std::optional<int> time = db.getLastTimestamp(topicHash);
            if (time.has_value()) { lastTimestamp = time.value(); }
            this->threads.push_back(std::make_unique<NtfyThread>(name, protocol, domain, topic, authConfig, lastTimestamp, &this->mutex));
        } catch (nlohmann::json::out_of_range e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
    }
}
