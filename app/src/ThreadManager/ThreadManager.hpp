#pragma once

#include <QObject>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "NtfyThread.hpp"
#include "../Config/Config.hpp"

class ThreadManager: public QObject {
    Q_OBJECT
    public:
        ThreadManager(QObject *parent = nullptr);
        ~ThreadManager();
    public slots:
        void stopAll();
        void restartConfig();
    private:
        std::vector<std::unique_ptr<NtfyThread>> threads;
        std::mutex mutex;
};
