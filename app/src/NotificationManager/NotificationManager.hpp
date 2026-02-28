#pragma once

#include "../NotificationManager/NtfyMessage.hpp"

#include <nlohmann/json.hpp>

#include <string>

namespace NotificationManager {
    void ntfy_notification(const NtfyMessage message);
    void general_notification(const QString title, const QString message);
    void startup_notification();
    void error_notification(const QString title, const QString message);
};
