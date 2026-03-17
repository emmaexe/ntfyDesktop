#include "SingleInstanceManager.hpp"

#include "../Util/Logging.hpp"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>

SingleInstanceManager* SingleInstanceManager::get() {
    static SingleInstanceManager* instance = new SingleInstanceManager(QApplication::instance());
    return instance;
}

void SingleInstanceManager::init(std::optional<QString> url) {
    std::call_once(this->init_flag, [&](){
        Logger& logger = Logger::instance();

        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.registerService("moe.emmaexe.ntfyDesktop")) {
            QDBusMessage message = QDBusMessage::createMethodCall("moe.emmaexe.ntfyDesktop", "/SingleInstanceManager", "moe.emmaexe.ntfyDesktop.SingleInstanceManager", "newInstance");
            message << url.has_value();
            message << (url.has_value() ? *url : "");
            QDBusMessage reply = sessionBus.call(message);
            if (reply.type() == QDBusMessage::ErrorMessage) { logger.error("DBus error: " + reply.errorMessage().toStdString()); }
            exit(0);
        }

        if (!sessionBus.registerObject("/SingleInstanceManager", this, QDBusConnection::ExportAllSlots)) {
            logger.error("Failed to register DBus object: " + sessionBus.lastError().message().toStdString());
            exit(0);
        }

        if (url.has_value()) { this->newInstance(true, *url); }
    });
}

SingleInstanceManager::SingleInstanceManager(QObject* parent): QObject(parent) {}

void SingleInstanceManager::newInstance(bool has_value, QString url) { emit new_instance(has_value ? std::make_optional(url) : std::nullopt); }
