#pragma once

#include <map>
#include <optional>
#include <string>

/**
 * @brief A static global class, holds the last seen notification.
 */
class NotificationStore {
    public:
        NotificationStore() = delete;
        /**
         * @brief Update a domain/topic with the ID of the last seen notification.
         *
         * @param domain The domain to update
         * @param topic The topic to update
         * @param notificationID The ID of the last seen notification for that domain/topic
         */
        static void update(const std::string& domain, const std::string& topic, const std::string& notificationID = "all");
        /**
         * @brief Get the last seen notification ID for some domain/topic.
         *
         * @param domain The domain to get
         * @param topic The topic to get
         * @return std::optional<std::string> - The ID of the last seen notification for that domain/topic. If there is no ID, returns `std::nullopt`.
         */
        static std::optional<std::string> get(const std::string& domain, const std::string& topic);
        /**
         * @brief Check if a last seen notification ID for some domain/topic exists.
         *
         * @param domain The domain to check
         * @param topic The topic to check
         */
        static bool exists(const std::string& domain, const std::string& topic);
        /**
         * @brief Remove a domain/topic from the NotificationStore.
         *
         * @param domain The domain to remove
         * @param topic The topic to remove
         */
        static void remove(const std::string& domain, const std::string& topic);
        /**
         * @brief Synchronize the NotificationStore to the Config.
         */
        static void configSync();
    private:
        static bool initialized;
        static std::map<std::string, std::string> internalData;
        static void read();
        static void write();
        static const std::string getStorePath();
        static const std::string getStoreFile();
};
