#pragma once

#include <QObject>
#include <functional>
#include <optional>

class SingleInstanceManager: public QObject {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "moe.emmaexe.ntfyDesktop.SingleInstanceManager")
    public:
        SingleInstanceManager(std::function<void(std::optional<std::string> url)> onNewInstanceStarted, std::optional<std::string> url, QObject* parent = nullptr);
        ~SingleInstanceManager();
        std::function<void(const std::optional<std::string> url)> onNewInstanceStarted;
    public slots:
        void newInstanceStarted(const QString& url);
};
