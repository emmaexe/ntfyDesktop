#pragma once

#include "../Config/Config.hpp"
#include "NtfyThread.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <QObject>
#include <vector>

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
