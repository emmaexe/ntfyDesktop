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

    std::optional<int> reconnectCount = std::make_optional(0), timeout = std::nullopt;
    try {
        nlohmann::json reconnectConfig = Config::data()["preferences"]["reconnect"];
        std::string modeStr = reconnectConfig["mode"];
        Util::Strings::toLower(modeStr);
        if (modeStr == "forever") {
            reconnectCount = std::nullopt;
        } else if (modeStr == "number") {
            reconnectCount = std::make_optional(reconnectConfig["numberValue"].get<int>());
        }
        std::string timeoutMode = reconnectConfig["timeoutMode"].get<std::string>();
        Util::Strings::toLower(timeoutMode);
        if (timeoutMode == "seconds") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000);
        } else if (timeoutMode == "minutes") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000*60);
        } else if (timeoutMode == "hours") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000*60*60);
        } else if (timeoutMode == "days") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000*60*60*24);
        } else if (timeoutMode == "weeks") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000*60*60*24*7);
        } else if (timeoutMode == "months") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000*60*60*24*30);
        } else if (timeoutMode == "years") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>()*1000*60*60*24*365);
        }
    } catch(const nlohmann::json::exception& ignored) {}

    DataBase db;
    for (nlohmann::json& source: Config::data()["sources"]) {
        try {
            std::string name = source["name"].get<std::string>(), protocol = source["protocol"].get<std::string>(), domain = source["domain"].get<std::string>(), topic = source["topic"].get<std::string>(), topicHash = Util::topicHash(domain, topic);
            AuthConfig authConfig = db.getAuth(topicHash);

            int lastTimestamp = -1;
            std::optional<int> time = db.getLastTimestamp(topicHash);
            if (time.has_value()) { lastTimestamp = time.value(); }

            bool verifyTls = db.getTlsVerificationPreference();
            std::string CAPath = db.getCAPathPreference();

            this->threads.push_back(std::make_unique<NtfyThread>(name, protocol, domain, topic, authConfig, lastTimestamp, verifyTls, CAPath, reconnectCount, timeout, &this->mutex));
        } catch (nlohmann::json::out_of_range e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
    }
}
