#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

enum NotificationAttachmentType { ICON, IMAGE };

struct NotificationAttachment {
    public:
        std::string name;
        std::string url;
        NotificationAttachmentType type;
        bool native;
};

enum NotificationActionType { CLICK, BUTTON };

struct NotificationAction {
    public:
        NotificationAction(const nlohmann::json& actionData);
        NotificationAction(const std::string& clickUrl);
        NotificationActionType type;
        bool useable;
        std::string label;
        std::string url;
};

enum NotificationPriority { LOWEST = 1, LOW = 2, NORMAL = 3, HIGH = 4, HIGHEST = 5 };

class NtfyNotificationException: public std::exception {
    public:
        NtfyNotificationException(const char* message);
        const char* what() const throw();
    private:
        std::string message;
};

struct NtfyNotification {
    public:
        NtfyNotification(const nlohmann::json notificationData, const std::string server, const std::string topic);
        NtfyNotification(const std::string rawData, const std::string topicHash);
        const std::string& rawData();
        const std::string& id();
        const std::string& topicHash();
        const int& time();
        const std::string& title();
        const std::string& message();
        const std::optional<NotificationPriority>& priority();
        const std::optional<NotificationAttachment>& attachment();
        const std::optional<std::vector<NotificationAction>>& actions();
    private:
        std::string internalRawData = "", internalId = "", internalTopicHash = "", internalTitle = "", internalMessage = "";
        int internalTime = 0;
        std::optional<NotificationPriority> internalPriority = std::nullopt;
        std::optional<NotificationAttachment> internalAttachment = std::nullopt;
        std::optional<std::vector<NotificationAction>> internalActions = std::nullopt;
};
