#include "ThreadManager.hpp"

#include "../DataBase/DataBase.hpp"
#include "../MainWindow/MainWindow.hpp"
#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Util.hpp"

#include <QApplication>

ThreadManager::ThreadManager(QObject* parent): QObject(parent) { this->recreateAll(); }

ThreadManager::~ThreadManager() { this->stopAll(); }

void ThreadManager::stopAll() {
    for (const NtfyWorker::Bundle& workerBundle: this->workerBundles) { workerBundle.worker->stop(); }
    for (const NtfyWorker::Bundle& workerBundle: this->workerBundles) { workerBundle.thread->wait(); }
    this->workerBundles.clear();
}

void ThreadManager::recreateAll() {
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
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000);
        } else if (timeoutMode == "minutes") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000 * 60);
        } else if (timeoutMode == "hours") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000 * 60 * 60);
        } else if (timeoutMode == "days") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000 * 60 * 60 * 24);
        } else if (timeoutMode == "weeks") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000 * 60 * 60 * 24 * 7);
        } else if (timeoutMode == "months") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000 * 60 * 60 * 24 * 30);
        } else if (timeoutMode == "years") {
            timeout = std::make_optional(reconnectConfig["timeoutValue"].get<int>() * 1000 * 60 * 60 * 24 * 365);
        }
    } catch (const nlohmann::json::exception& ignored) {}

    DataBase db;
    for (nlohmann::json& source: Config::data()["sources"]) {
        try {
            std::string domain = source["domain"].get<std::string>(), topic = source["topic"].get<std::string>();
            std::string topicHash = Util::topicHash(domain, topic);

            NtfyWorker::ConnectionOptions options{
                source["name"].get<std::string>(),
                source["protocol"].get<std::string>(),
                domain,
                topic,
                db.getAuth(topicHash),
                db.getLastTimestamp(topicHash),
                reconnectCount,
                timeout,
                db.getCAPathPreference(),
                db.getTlsVerificationPreference()
            };

            std::unique_ptr<NtfyWorker::BaseWorker> worker = std::make_unique<NtfyWorker::NtfyWorker>(options);
            std::unique_ptr<QThread> thread = std::make_unique<QThread>();

            worker->moveToThread(thread.get());

            QObject::connect(thread.get(), &QThread::started, worker.get(), &NtfyWorker::NtfyWorker::run);
            QObject::connect(worker.get(), &NtfyWorker::NtfyWorker::finished, [thread = thread.get()](NtfyWorker::ExitData exit) {
                if (exit.fatalError && !exit.errors.empty()) {
                    const std::string title = "Unable to connect to " + exit.options.workerName;
                    const std::string message = "Maximum retries exceeded.";
                    QMetaObject::invokeMethod(QApplication::instance(), [title, message]() { NotificationManager::errorNotification(title, message); });
                }
                thread->quit();
            });

            thread->start();

            this->workerBundles.push_back(NtfyWorker::Bundle{ std::move(worker), std::move(thread) });
        } catch (nlohmann::json::out_of_range e) { Logger::get().error("Invalid source in config, ignoring: " + source.dump()); }
    }
}
