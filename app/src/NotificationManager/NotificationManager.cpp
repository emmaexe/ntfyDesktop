#include "NotificationManager.hpp"

#include <QDesktopServices>
#include <KNotification>

#include "../FileManager/FileManager.hpp"

NotificationAction::NotificationAction(const nlohmann::json& actionData) {
    this->type = NotificationActionType::BUTTON;
    try {
        if (actionData["action"] == "view") {
            this->label = actionData["label"];
            this->url = actionData["url"];
            this->useable = true;
        } else {
            this->useable = false;
        }
    } catch (nlohmann::json::parse_error err) {
        this->useable = false;
    }
}

NotificationAction::NotificationAction(const std::string& clickUrl) {
    this->type = NotificationActionType::CLICK;
    this->label = "Open URL";
    this->url = clickUrl;
    this->useable = true;
}

void NotificationManager::generalNotification(const std::string title, const std::string message, std::optional<NotificationPriority> priority, std::optional<NotificationAttachment> attachment, std::optional<std::vector<NotificationAction>> actions) {
    KNotification* notification = new KNotification("general");

    if (priority.has_value()) {
        if (priority.value() == NotificationPriority::HIGHEST) {
            notification->setUrgency(KNotification::Urgency::CriticalUrgency);
        } else if (priority.value() == NotificationPriority::HIGH) {
            notification->setUrgency(KNotification::Urgency::HighUrgency);
        } else if (priority.value() == NotificationPriority::NORMAL) {
            notification->setUrgency(KNotification::Urgency::NormalUrgency);
        } else if (priority.value() == NotificationPriority::LOW || priority.value() == NotificationPriority::LOWEST) {
            notification->setUrgency(KNotification::Urgency::LowUrgency);
        }
    }

    notification->setTitle(title.c_str());
    notification->setText(message.c_str());
    notification->setIconName("ntfyDesktop");

    if (attachment.has_value()) {
        QUrl tempFileUrl = FileManager::urlToTempFile(QUrl(QString::fromStdString(attachment->url)));
        notification->setUrls(QList<QUrl>({ tempFileUrl }));
    }

    if (actions.has_value()) {
        for (NotificationAction action: actions.value()) {
            if (!action.useable) { continue; }
            KNotificationAction* knaction;

            if (action.type == NotificationActionType::CLICK) {
                knaction = notification->addDefaultAction(QString::fromStdString(action.label));
            } else if (action.type == NotificationActionType::BUTTON) {
                knaction = notification->addAction(QString::fromStdString(action.label));
            }

            KNotificationAction::connect(knaction, &KNotificationAction::activated, [actionUrl = action.url](){
                QDesktopServices::openUrl(QUrl(QString::fromStdString(actionUrl)));
            });
        }
    }

    notification->sendEvent();
}

void NotificationManager::startupNotification() {
    KNotification* notification = new KNotification("startup");
    notification->setUrgency(KNotification::Urgency::LowUrgency);
    notification->setTitle("Ntfy Desktop");
    notification->setText("Ntfy Desktop is running in the background.");
    notification->setIconName("ntfyDesktop");
    notification->sendEvent();
}
