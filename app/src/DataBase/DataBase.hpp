#pragma once

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <string>

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
        AuthConfig getAuth(const std::string& topicHash);

        void executeQuery(const std::string& query);
    private:
        QSqlDatabase db;
        std::string connectionName = "SQliteDB";
        static const std::string getDBPath();
        static const std::string getDBFile();
};
