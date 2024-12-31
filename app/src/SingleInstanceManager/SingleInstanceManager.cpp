#include "SingleInstanceManager.hpp"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <iostream>

SingleInstanceManager::SingleInstanceManager(std::function<void(std::optional<std::string> url)> onNewInstanceStarted, std::optional<std::string> url, QObject* parent):
    QObject(parent), onNewInstanceStarted(onNewInstanceStarted) {
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService("moe.emmaexe.ntfyDesktop")) {
        QDBusMessage message = QDBusMessage::createMethodCall("moe.emmaexe.ntfyDesktop", "/SingleInstanceManager", "moe.emmaexe.ntfyDesktop.SingleInstanceManager", "newInstanceStarted");
        if (url.has_value()) {
            message << QString::fromStdString(url.value());
        } else {
            message << "";
        }
        QDBusMessage reply = sessionBus.call(message);
        if (reply.type() == QDBusMessage::ErrorMessage) { std::cerr << "DBus error: " << reply.errorMessage().toStdString() << std::endl; }
        exit(1);
    }

    if (!sessionBus.registerObject("/SingleInstanceManager", this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        std::cerr << "Failed to register DBus object: " << sessionBus.lastError().message().toStdString() << std::endl;
        exit(1);
    }

    if (url.has_value()) { this->newInstanceStarted(QString::fromStdString(url.value())); }
}

SingleInstanceManager::~SingleInstanceManager() {}

void SingleInstanceManager::newInstanceStarted(const QString& url) {
    if (this->onNewInstanceStarted) { this->onNewInstanceStarted(url.isEmpty() ? std::nullopt : std::make_optional(url.toStdString())); }
}
