#pragma once

#include "../Config/Config.hpp"
#include "../ThreadManager/NtfyWorker.hpp"

#include <QObject>
#include <QThread>
#include <vector>

class ThreadManager: public QObject {
        Q_OBJECT
    public:
        ThreadManager(QObject *parent = nullptr);
        ~ThreadManager();
    public slots:
        void stopAll();
        void recreateAll();
    private:
        std::vector<NtfyWorker::Bundle> workerBundles;
};
