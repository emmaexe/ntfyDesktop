#pragma once

#include "./NtfyNotification.hpp"

#include <nlohmann/json.hpp>

#include <string>

class NotificationManager {
    public:
        NotificationManager() = delete;
        static void generalNotification(NtfyNotification ntfyNotification);
        static void startupNotification();
        static void errorNotification(const std::string title, const std::string message);
};
