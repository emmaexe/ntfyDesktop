#include "NotificationManager.hpp"

#include <QDesktopServices>
#include <KNotification>
#include <iostream>

NotificationAction::NotificationAction(const nlohmann::json& actionData) {
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
    this->label = "Open URL";
    this->url = clickUrl;
    this->useable = true;
}

void NotificationManager::generalNotification(const std::string& title, const std::string& message, std::optional<NotificationPriority> priority, std::optional<NotificationAttachment> attachment, std::optional<std::vector<NotificationAction>> actions) {
    KNotification* notification = new KNotification("general");
    notification->setTitle(title.c_str());
    notification->setText(message.c_str());
    notification->setIconName("ntfyDesktop");
    if (priority.has_value()) {
        // TODO: priority system
    }
    if (attachment.has_value()) {
        // TODO: Check if attachment is native to the ntfy server, or if it is external and proxy it (?)
        notification->setUrls(QList<QUrl>({ QUrl(attachment->url.c_str()) }));
    }
    if (actions.has_value()) {
        for (NotificationAction action: actions.value()) {
            if (!action.useable) { continue; }
            KNotificationAction* knaction = notification->addAction(action.label.c_str());
            KNotificationAction::connect(knaction, &KNotificationAction::activated, [&](){
                QDesktopServices::openUrl(QUrl(action.url.c_str()));
            });
        }
    }
    notification->sendEvent();
}
