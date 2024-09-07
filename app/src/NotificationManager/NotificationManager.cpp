#include "NotificationManager.hpp"

#include <QDesktopServices>
#include <KNotification>
#include <iostream>

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
    notification->setTitle(title.c_str());
    notification->setText(message.c_str());
    notification->setIconName("ntfyDesktop");
    if (priority.has_value()) {
        // TODO: priority system
    }
    if (attachment.has_value()) {
        // TODO: Check if attachment is native to the ntfy server, or if it is external and proxy it (?)
        notification->setUrls(QList<QUrl>({ QUrl(QString::fromStdString(attachment->url)) }));
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
