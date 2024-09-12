#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

enum NotificationPriority {
    LOWEST = 1,
    LOW = 2,
    NORMAL = 3,
    HIGH = 4,
    HIGHEST = 5
};

enum NotificationAttachmentType {
    ICON,
    IMAGE
};

struct NotificationAttachment {
    std::string name;
    std::string url;
    NotificationAttachmentType type;
    bool native;
};

enum NotificationActionType {
    CLICK,
    BUTTON
};

class NotificationAction {
    public:
        NotificationAction(const nlohmann::json& actionData);
        NotificationAction(const std::string& clickUrl);
        NotificationActionType type;
        bool useable;
        std::string label;
        std::string url;
};

class NotificationManager {
    public:
        NotificationManager() = delete;
        static void generalNotification(const std::string title, const std::string message, std::optional<NotificationPriority> priority = std::nullopt, std::optional<NotificationAttachment> attachment = std::nullopt, std::optional<std::vector<NotificationAction>> actions = std::nullopt);
        static void startupNotification();
};
