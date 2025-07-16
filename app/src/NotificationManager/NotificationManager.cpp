#include "NotificationManager.hpp"

#include "../Config/Config.hpp"
#include "../Util/FileManager.hpp"
#include "ntfyDesktop.hpp"

#include <KNotification>
#include <QDesktopServices>

void NotificationManager::generalNotification(NtfyNotification ntfyNotification) {
    KNotification* notification = new KNotification("general");

    if (ntfyNotification.priority().has_value()) {
        if (ntfyNotification.priority().value() == NotificationPriority::HIGHEST) {
            notification->setUrgency(KNotification::Urgency::CriticalUrgency);
        } else if (ntfyNotification.priority().value() == NotificationPriority::HIGH) {
            notification->setUrgency(KNotification::Urgency::HighUrgency);
        } else if (ntfyNotification.priority().value() == NotificationPriority::NORMAL) {
            notification->setUrgency(KNotification::Urgency::NormalUrgency);
        } else if (ntfyNotification.priority().value() == NotificationPriority::LOW || ntfyNotification.priority().value() == NotificationPriority::LOWEST) {
            notification->setUrgency(KNotification::Urgency::LowUrgency);
        }
    }

    notification->setTitle(QString::fromStdString(ntfyNotification.title()));
    notification->setText(QString::fromStdString(ntfyNotification.message()));
    notification->setIconName("moe.emmaexe.ntfyDesktop");

    if (ntfyNotification.attachment().has_value()) {
        if (ND_BUILD_TYPE == "Flatpak") {
            QUrl fileUrl = QUrl(QString::fromStdString(ntfyNotification.attachment()->url));
            KNotificationAction* knaction = notification->addAction(QStringLiteral("Open Attachment"));
            KNotificationAction::connect(knaction, &KNotificationAction::activated, [fileUrl]() { QDesktopServices::openUrl(fileUrl); });
        } else {
            QUrl tempFileUrl = FileManager::urlToTempFile(QUrl(QString::fromStdString(ntfyNotification.attachment()->url)));
            notification->setUrls({ tempFileUrl });
        }
    }

    if (ntfyNotification.actions().has_value()) {
        for (NotificationAction action: ntfyNotification.actions().value()) {
            if (!action.useable) { continue; }
            KNotificationAction* knaction;

            if (action.type == NotificationActionType::CLICK) {
                KNotificationAction* bknaction = notification->addAction(QString::fromStdString(action.label));
                KNotificationAction::connect(bknaction, &KNotificationAction::activated, [actionUrl = action.url]() { QDesktopServices::openUrl(QUrl(QString::fromStdString(actionUrl))); });
                knaction = notification->addDefaultAction(QString::fromStdString(action.label));
            } else if (action.type == NotificationActionType::BUTTON) {
                knaction = notification->addAction(QString::fromStdString(action.label));
            }

            KNotificationAction::connect(knaction, &KNotificationAction::activated, [actionUrl = action.url]() { QDesktopServices::openUrl(QUrl(QString::fromStdString(actionUrl))); });
        }
    }

    notification->sendEvent();
}

void NotificationManager::startupNotification() {
    if (Config::data()["preferences"].is_object() && Config::data()["preferences"]["notifications"].is_object() && Config::data()["preferences"]["notifications"]["startup"].is_boolean() && Config::data()["preferences"]["notifications"]["startup"]) {
        KNotification* notification = new KNotification("startup");
        notification->setUrgency(KNotification::Urgency::LowUrgency);
        notification->setTitle("Ntfy Desktop");
        notification->setText("Ntfy Desktop is running in the background.");
        notification->setIconName("moe.emmaexe.ntfyDesktop");
        notification->sendEvent();
    }
}

void NotificationManager::errorNotification(const std::string title, const std::string message) {
    if (Config::data()["preferences"].is_object() && Config::data()["preferences"]["notifications"].is_object() && Config::data()["preferences"]["notifications"]["error"].is_boolean() && Config::data()["preferences"]["notifications"]["error"]) {
        KNotification* notification = new KNotification("error");
        notification->setTitle(QString::fromStdString(title));
        notification->setText(QString::fromStdString(message));
        notification->setUrgency(KNotification::Urgency::HighUrgency);
        notification->setIconName(QStringLiteral("moe.emmaexe.ntfyDesktop"));
        notification->sendEvent();
    }
}
