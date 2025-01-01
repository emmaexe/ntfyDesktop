#include "NtfyNotification.hpp"

#include "../Util/Util.hpp"

#include <emojicpp/emoji.hpp>

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
    } catch (nlohmann::json::parse_error err) { this->useable = false; }
}

NotificationAction::NotificationAction(const std::string& clickUrl) {
    this->type = NotificationActionType::CLICK;
    this->label = "Open URL";
    this->url = clickUrl;
    this->useable = true;
}

NtfyNotificationException::NtfyNotificationException(const char* message): message(message) {}

const char* NtfyNotificationException::what() const throw() { return this->message.c_str(); }

NtfyNotification::NtfyNotification(const nlohmann::json notificationData, const std::string server, const std::string topic) {
    try {
        this->internalRawData = notificationData.dump();
        this->internalId = notificationData["id"];
        this->internalTopicHash = Util::topicHash(server, topic);
        this->internalTime = notificationData["time"].get<int>();

        this->internalTitle = notificationData.contains("title") ? notificationData["title"] : notificationData["topic"];
        this->internalMessage = notificationData["message"];

        if (notificationData.contains("tags")) {
            bool seenTag = false;
            for (nlohmann::json item: notificationData["tags"]) {
                std::string tag = static_cast<std::string>(item);
                std::string ptag = emojicpp::emoji::parse(":" + tag + ":");
                if (":" + tag + ":" == ptag) {
                    if (!seenTag) {
                        seenTag = true;
                        this->internalMessage += " Tags: ";
                    }
                    this->internalMessage += tag + " ";
                } else {
                    this->internalTitle = ptag + " " + this->internalTitle;
                }
            }
        }

        if (notificationData.contains("priority")) { this->internalPriority = static_cast<NotificationPriority>(notificationData["priority"].get<int>()); }

        if (notificationData.contains("icon")) {
            this->internalAttachment = NotificationAttachment();
            this->internalAttachment->type = NotificationAttachmentType::ICON;
            this->internalAttachment->url = notificationData["icon"];
        }
        if (notificationData.contains("attachment")) {
            this->internalAttachment = NotificationAttachment();
            this->internalAttachment->type = NotificationAttachmentType::IMAGE;
            this->internalAttachment->name = notificationData["attachment"]["name"];
            this->internalAttachment->url = notificationData["attachment"]["url"];
            this->internalAttachment->native = notificationData["attachment"].contains("type");
        }

        if (notificationData.contains("click")) {
            if (!this->internalActions.has_value()) { this->internalActions = std::vector<NotificationAction>({}); }
            this->internalActions->push_back(NotificationAction(static_cast<std::string>(notificationData["click"])));
        }
        if (notificationData.contains("actions")) {
            if (!this->internalActions.has_value()) { this->internalActions = std::vector<NotificationAction>({}); }
            for (nlohmann::json element: notificationData["actions"]) { this->internalActions->push_back(NotificationAction(element)); }
        }
    } catch (nlohmann::json::parse_error e) {
        const std::string err = "Failed to parse notification: " + static_cast<std::string>(e.what());
        throw NtfyNotificationException(err.c_str());
    }
}

NtfyNotification::NtfyNotification(const std::string rawData, const std::string topicHash): internalRawData(rawData), internalTopicHash(topicHash) {
    try {
        nlohmann::json notificationData = nlohmann::json::parse(rawData);

        this->internalId = notificationData["id"];
        this->internalTime = notificationData["time"].get<int>();

        this->internalTitle = notificationData.contains("title") ? notificationData["title"] : notificationData["topic"];
        this->internalMessage = notificationData["message"];

        if (notificationData.contains("tags")) {
            bool seenTag = false;
            for (nlohmann::json item: notificationData["tags"]) {
                std::string tag = static_cast<std::string>(item);
                std::string ptag = emojicpp::emoji::parse(":" + tag + ":");
                if (":" + tag + ":" == ptag) {
                    if (!seenTag) {
                        seenTag = true;
                        this->internalMessage += " Tags: ";
                    }
                    this->internalMessage += tag + " ";
                } else {
                    this->internalTitle = ptag + " " + this->internalTitle;
                }
            }
        }

        if (notificationData.contains("priority")) { this->internalPriority = static_cast<NotificationPriority>(notificationData["priority"].get<int>()); }

        if (notificationData.contains("icon")) {
            this->internalAttachment = NotificationAttachment();
            this->internalAttachment->type = NotificationAttachmentType::ICON;
            this->internalAttachment->url = notificationData["icon"];
        }
        if (notificationData.contains("attachment")) {
            this->internalAttachment = NotificationAttachment();
            this->internalAttachment->type = NotificationAttachmentType::IMAGE;
            this->internalAttachment->name = notificationData["attachment"]["name"];
            this->internalAttachment->url = notificationData["attachment"]["url"];
            this->internalAttachment->native = notificationData["attachment"].contains("type");
        }

        if (notificationData.contains("click")) {
            if (!this->internalActions.has_value()) { this->internalActions = std::vector<NotificationAction>({}); }
            this->internalActions->push_back(NotificationAction(static_cast<std::string>(notificationData["click"])));
        }
        if (notificationData.contains("actions")) {
            if (!this->internalActions.has_value()) { this->internalActions = std::vector<NotificationAction>({}); }
            for (nlohmann::json element: notificationData["actions"]) { this->internalActions->push_back(NotificationAction(element)); }
        }
    } catch (nlohmann::json::parse_error e) {
        const std::string err = "Failed to parse notification: " + static_cast<std::string>(e.what());
        throw NtfyNotificationException(err.c_str());
    }
}

const std::string& NtfyNotification::rawData() { return this->internalRawData; }
const std::string& NtfyNotification::id() { return this->internalId; }
const std::string& NtfyNotification::topicHash() { return this->internalTopicHash; }
const int& NtfyNotification::time() { return this->internalTime; }
const std::string& NtfyNotification::title() { return this->internalTitle; }
const std::string& NtfyNotification::message() { return this->internalMessage; }
const std::optional<NotificationPriority>& NtfyNotification::priority() { return this->internalPriority; }
const std::optional<NotificationAttachment>& NtfyNotification::attachment() { return this->internalAttachment; }
const std::optional<std::vector<NotificationAction>>& NtfyNotification::actions() { return this->internalActions; }
