#pragma once

#include "../NotificationManager/NtfyNotification.hpp"
#include "../HistoryDialog/HistoryDialog.hpp"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <string>
#include <optional>

enum AuthType {
    NONE, USERNAME_PASSWORD, TOKEN
};

struct AuthConfig {
    AuthType type = AuthType::NONE;
    std::string username = "", password = "", token = "";
};

class DataBase {
    public:
        DataBase();
        ~DataBase();

        void setAuth(const std::string& topicHash, const AuthConfig& authConfig);
        void multiSetAuth(const std::map<std::string, AuthConfig>& data);
        AuthConfig getAuth(const std::string& topicHash);

        void enqueueNotification(const NtfyNotification notification);
        void commitNotificationQueue();
        const std::optional<NtfyNotification> getLastNotification(const std::string& topicHash);
        std::vector<std::unique_ptr<NotificationListItem>> getAllNotifications(QWidget* parent = nullptr);

        void executeQuery(const std::string& query);
        bool hasRows(const std::string& table);
        int countRows(const std::string& table);
    private:
        QSqlDatabase db;
        std::vector<NtfyNotification> notificationQueue = {};
        std::string connectionName = "SQliteDB";
        static const std::string getDBPath();
        static const std::string getDBFile();
};
