#include "DataBase.hpp"

#include "../Config/Config.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <QStandardPaths>
#include <QUuid>
#include <QtSql/QSqlError>
#include <iostream>

DataBase::DataBase() {
    this->connectionName = "SQliteDB_" + QUuid::createUuid().toString(QUuid::Id128).toStdString();
    this->db = QSqlDatabase::addDatabase("QSQLITE", QString::fromStdString(this->connectionName));
    this->db.setDatabaseName(QString::fromStdString(DataBase::getDBFile()));
    this->db.open();

    QSqlQuery query(this->db);
    this->db.transaction();
    if (
        query.exec("PRAGMA auto_vacuum = FULL;") &&
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS Meta (
                Key TEXT NOT NULL PRIMARY KEY,
                Value TEXT NOT NULL
            );
        )") &&
        query.exec(
            "INSERT OR REPLACE INTO meta (key, value)"
            "VALUES ('version', '" ND_VERSION "');"
        ) &&
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS Auth (
                TopicHash TEXT NOT NULL PRIMARY KEY,
                AuthType INT NOT NULL,
                Username TEXT,
                Password TEXT,
                Token TEXT
            );
        )") &&
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS Notifications (
                Id TEXT NOT NULL PRIMARY KEY,
                TopicHash TEXT NOT NULL,
                Time INT NOT NULL,
                Title TEXT NOT NULL,
                Message TEXT NOT NULL,
                RawData TEXT NOT NULL
            );
        )")
    ) {
        this->db.commit();
    } else {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
        this->db.rollback();
    }
}

DataBase::~DataBase() {
    if (!this->notificationQueue.empty()) { this->commitNotificationQueue(); }
    if (this->db.isOpen()) { this->db.close(); }
    this->db = QSqlDatabase(); // Qt logs unhappy messages when there is a QSqlDatabase object still connected to a db that is being removed.
    QSqlDatabase::removeDatabase(QString::fromStdString(this->connectionName));
}

void DataBase::setAuth(const std::string& topicHash, const AuthConfig& authConfig) {
    QSqlQuery query(this->db);
    if (authConfig.type == AuthType::NONE) {
        query.prepare(R"(
            INSERT OR REPLACE INTO Auth (TopicHash, AuthType)
            VALUES (?, ?);
        )");
        query.addBindValue(QString::fromStdString(topicHash));
        query.addBindValue(authConfig.type);
    } else if (authConfig.type == AuthType::USERNAME_PASSWORD) {
        query.prepare(R"(
            INSERT OR REPLACE INTO Auth (TopicHash, AuthType, Username, Password)
            VALUES (?, ?, ?, ?);
        )");
        query.addBindValue(QString::fromStdString(topicHash));
        query.addBindValue(authConfig.type);
        query.addBindValue(QString::fromStdString(authConfig.username));
        query.addBindValue(QString::fromStdString(authConfig.password));
    } else if (authConfig.type == AuthType::TOKEN) {
        query.prepare(R"(
            INSERT OR REPLACE INTO Auth (TopicHash, AuthType, Token)
            VALUES (?, ?, ?);
        )");
        query.addBindValue(QString::fromStdString(topicHash));
        query.addBindValue(authConfig.type);
        query.addBindValue(QString::fromStdString(authConfig.token));
    }
    if (!query.exec()) { std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl; }
}

void DataBase::multiSetAuth(const std::map<std::string, AuthConfig>& data) {
    QSqlQuery query(this->db);
    this->db.transaction();
    bool failure = false;

    failure = failure ? true : !query.exec(R"(
        DELETE FROM Auth;
    )");

    for (const std::pair<std::string, AuthConfig>& topic: data) {
        if (failure) { break; }

        if (topic.second.type == AuthType::NONE) {
            query.prepare(R"(
                INSERT OR REPLACE INTO Auth (TopicHash, AuthType)
                VALUES (:topic_hash, :auth_type);
            )");
            query.bindValue(":topic_hash", QString::fromStdString(topic.first));
            query.bindValue(":auth_type", topic.second.type);
        } else if (topic.second.type == AuthType::USERNAME_PASSWORD) {
            query.prepare(R"(
                INSERT OR REPLACE INTO Auth (TopicHash, AuthType, Username, Password)
                VALUES (:topic_hash, :auth_type, :username, :password);
            )");
            query.bindValue(":topic_hash", QString::fromStdString(topic.first));
            query.bindValue(":auth_type", topic.second.type);
            query.bindValue(":username", QString::fromStdString(topic.second.username));
            query.bindValue(":password", QString::fromStdString(topic.second.password));
            failure = failure ? true : !query.exec();
        } else if (topic.second.type == AuthType::TOKEN) {
            query.prepare(R"(
                INSERT OR REPLACE INTO Auth (TopicHash, AuthType, Token)
                VALUES (:topic_hash, :auth_type, :token)
            )");
            query.bindValue(":topic_hash", QString::fromStdString(topic.first));
            query.bindValue(":auth_type", topic.second.type);
            query.bindValue(":token", QString::fromStdString(topic.second.token));
        }

        failure = failure ? true : !query.exec();
        query.clear();
    }

    if (!failure) {
        this->db.commit();
    } else {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
        this->db.rollback();
    }
}

AuthConfig DataBase::getAuth(const std::string& topicHash) {
    AuthConfig authConfig;
    authConfig.type = AuthType::NONE;

    QSqlQuery query(this->db);
    query.prepare(R"(
        SELECT AuthType, Username, Password, Token
        FROM Auth
        WHERE TopicHash = ?
    )");
    query.addBindValue(QString::fromStdString(topicHash));

    if (query.exec()) {
        if (query.next()) {
            authConfig.type = static_cast<AuthType>(query.value(0).toInt());
            authConfig.username = query.value(1).isNull() ? "" : query.value(1).toString().toStdString();
            authConfig.password = query.value(2).isNull() ? "" : query.value(2).toString().toStdString();
            authConfig.token = query.value(3).isNull() ? "" : query.value(3).toString().toStdString();
        }
    } else {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
    }

    return authConfig;
}

void DataBase::enqueueNotification(const NtfyNotification notification) { this->notificationQueue.push_back(notification); }

void DataBase::commitNotificationQueue() {
    QSqlQuery query(this->db);
    this->db.transaction();
    bool failure = false;

    for (NtfyNotification notification: this->notificationQueue) {
        if (failure) { break; }

        query.prepare(R"(
            INSERT OR REPLACE INTO Notifications (Id, TopicHash, Time, Title, Message, RawData)
            VALUES (:id, :topic_hash, :time, :title, :message, :rawdata)
        )");
        query.bindValue(":id", QString::fromStdString(notification.id()));
        query.bindValue(":topic_hash", QString::fromStdString(notification.topicHash()));
        query.bindValue(":time", notification.time());
        query.bindValue(":title", QString::fromStdString(notification.title()));
        query.bindValue(":message", QString::fromStdString(notification.message()));
        query.bindValue(":rawdata", QString::fromStdString(notification.rawData()));

        failure = failure ? true : !query.exec();
        query.clear();
    }

    if (!failure) {
        this->db.commit();
    } else {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
        this->db.rollback();
    }
    this->notificationQueue.clear();
}

const std::optional<NtfyNotification> DataBase::getLastNotification(const std::string& topicHash) {
    QSqlQuery query(this->db);
    query.prepare(R"(
        SELECT RawData
        FROM Notifications
        WHERE TopicHash = :topic_hash
        ORDER BY Time DESC
        LIMIT 1
    )");
    query.bindValue(":topic_hash", QString::fromStdString(topicHash));

    if (query.exec() && query.next()) { return std::make_optional<NtfyNotification>(NtfyNotification(query.value(0).toString().toStdString(), topicHash)); }
    return std::nullopt;
}

std::vector<std::unique_ptr<NotificationListItem>> DataBase::getNotificationsAsListItem(QWidget* parent) {
    std::vector<std::unique_ptr<NotificationListItem>> notifications = {};
    QSqlQuery query(this->db);
    query.prepare(R"(
        SELECT Id, TopicHash, Time, Title, Message, RawData
        FROM Notifications
        ORDER BY Time DESC
    )");
    if (query.exec()) {
        std::map<std::string, std::pair<std::string, std::string>> hashes = {};
        nlohmann::json sources = Config::data()["sources"];
        for (int i = 0; i < sources.size(); i++) {
            std::string server = sources[i]["domain"];
            std::string topic = sources[i]["topic"];
            hashes.emplace(std::make_pair(Util::topicHash(server, topic), std::make_pair(server, topic)));
        }

        while (query.next()) {
            const std::string id = query.value(0).toString().toStdString();
            const std::string topicHash = query.value(1).toString().toStdString();
            const std::string server = hashes[topicHash].first, topic = hashes[topicHash].second;
            const int timestamp = query.value(2).toInt();
            const std::string title = query.value(3).toString().toStdString();
            const std::string message = query.value(4).toString().toStdString();
            const std::string rawData = query.value(5).toString().toStdString();

            std::unique_ptr<NotificationListItem> notification = std::make_unique<NotificationListItem>(id, server, topic, timestamp, title, message, rawData, parent);
            if (notification->valid()) { notifications.push_back(std::move(notification)); }
        }
    }
    return notifications;
}

void DataBase::deleteNotification(const std::string& id) {
    QSqlQuery query(this->db);
    query.prepare(R"(
        DELETE FROM Notifications
        WHERE Id = :id
    )");
    query.bindValue(":id", QString::fromStdString(id));
    if (!query.exec()) { std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl; }
}

void DataBase::deleteNotifications(const std::vector<std::string>& ids) {
    QSqlQuery query(this->db);
    this->db.transaction();
    bool failure = false;

    for (std::string id: ids) {
        if (failure) { break; }
        query.prepare(R"(
            DELETE FROM Notifications
            WHERE Id = :id
        )");
        query.bindValue(":id", QString::fromStdString(id));

        failure = failure ? true : !query.exec();
        query.clear();
    }

    if (!failure) {
        this->db.commit();
    } else {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
        this->db.rollback();
    }
}

bool DataBase::hasRows(const std::string& table) {
    QSqlQuery query(this->db);
    query.prepare(QString(R"(
        SELECT 1
        FROM %1
        LIMIT 1
    )").arg(QString::fromStdString(table)));
    if (query.exec() && query.next()) { return true; }
    return false;
}

int DataBase::countRows(const std::string& table) {
    QSqlQuery query(this->db);
    query.prepare(QString(R"(
        SELECT COUNT(*)
        FROM %1
    )").arg(QString::fromStdString(table)));
    if (query.exec() && query.next()) { return query.value(0).toInt(); }
    return 0;
}

void DataBase::executeQuery(const std::string& queryText) {
    QSqlQuery query(QString::fromStdString(queryText), this->db);
    if (!query.exec()) { std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl; }
}

const std::string DataBase::getDBPath() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString(); }

const std::string DataBase::getDBFile() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + std::string("/db.sqlite3"); }
