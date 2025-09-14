#include "DataBase.hpp"

#include "../Config/Config.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"

#include <nlohmann/json.hpp>

#include <QStandardPaths>
#include <QUuid>
#include <QtSql/QSqlError>
#include <iostream>
#include <filesystem>

DataBase::DataBase() {
    if (!std::filesystem::exists(DataBase::getDBPath()) || !std::filesystem::is_directory(DataBase::getDBPath())) { std::filesystem::create_directory(DataBase::getDBPath()); }

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
            CREATE TABLE IF NOT EXISTS GlobalPreferences (
                Key TEXT NOT NULL PRIMARY KEY,
                Value TEXT NOT NULL
            );
        )") &&
        query.exec(
            "INSERT INTO GlobalPreferences (key, value)"
            "VALUES ('TlsVerification', '1'),"
            "       ('CaPath', '')"
            "ON CONFLICT(key) DO NOTHING;"
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
        )") &&
        query.exec(R"(
            CREATE TABLE IF NOT EXISTS LastNotifications (
                TopicHash TEXT NOT NULL PRIMARY KEY,
                Time INT NOT NULL
            );
        )") &&
        query.exec(R"(
            CREATE INDEX IF NOT EXISTS IDX_TIME ON Notifications(Time);
        )") &&
        query.exec(R"(
            CREATE INDEX IF NOT EXISTS IDX_TOPICHASH ON Notifications(TopicHash);
        )") &&
        query.exec(R"(
            CREATE TRIGGER IF NOT EXISTS TimestampUpdate
            BEFORE INSERT ON Notifications
            BEGIN
                INSERT INTO LastNotifications (TopicHash, Time)
                VALUES (NEW.TopicHash, NEW.Time)
                ON CONFLICT(TopicHash) DO UPDATE SET Time =
                    CASE
                        WHEN NEW.Time > LastNotifications.Time THEN NEW.Time
                        ELSE LastNotifications.Time
                    END;
            END;
        )") &&
        query.exec(R"(
            DROP TRIGGER IF EXISTS NotificationsLimit;
        )")
    ) {
        const std::array<int, 7> valueMap = { 1, 60, 3600, 86400, 604800, 2592000, 31536000 };
        int mode = 0, numberValue = 5000, recentValue = 2, recentMode = 4, sourceMode = 0;
        bool failure = false;
        try {
            nlohmann::json historyConfig = Config::data()["preferences"]["history"];
            if (historyConfig.is_object() && historyConfig["mode"].is_string()) {
                std::string configMode = historyConfig["mode"].get<std::string>();
                Util::Strings::toLower(configMode);
                if (configMode == "all") {
                    mode = 0;
                } else if (configMode == "number" && historyConfig["numberValue"].is_number()) {
                    mode = 1;
                    numberValue = historyConfig["numberValue"].get<int>();
                } else if (configMode == "recent" && historyConfig["recentValue"].is_number() && historyConfig["recentMode"].is_string()) {
                    mode = 2;
                    recentValue = historyConfig["recentValue"].get<int>();
                    std::string configRecentMode = historyConfig["recentMode"].get<std::string>();
                    Util::Strings::toLower(configRecentMode);
                    if (configRecentMode == "seconds") {
                        recentMode = 0;
                    } else if (configRecentMode == "minutes") {
                        recentMode = 1;
                    } else if (configRecentMode == "hours") {
                        recentMode = 2;
                    } else if (configRecentMode == "days") {
                        recentMode = 3;
                    } else if (configRecentMode == "weeks") {
                        recentMode = 4;
                    } else if (configRecentMode == "months") {
                        recentMode = 5;
                    } else if (configRecentMode == "years") {
                        recentMode = 6;
                    }
                } else if (configMode == "none") {
                    mode = 3;
                }
                if (historyConfig["sourceMode"].is_string()) {
                    std::string modeStr = historyConfig["sourceMode"].get<std::string>();
                    Util::Strings::toLower(modeStr);
                    if (modeStr == "individual") {
                        sourceMode = 0;
                    } else if (modeStr == "all") {
                        sourceMode = 1;
                    }
                }
            }
        } catch (const nlohmann::json::type_error& ignored) {}
        recentValue *= valueMap[recentMode];
        if (mode == 1) {
            if (sourceMode == 0) {
                query.prepare(QString(R"(
                    CREATE TRIGGER NotificationsLimit
                    AFTER INSERT ON Notifications
                    WHEN (SELECT COUNT(*) FROM Notifications) > %1
                    BEGIN
                        DELETE FROM Notifications
                        WHERE Id IN (
                            SELECT Id FROM Notifications
                            WHERE TopicHash = NEW.TopicHash
                            AND Id NOT IN (
                                SELECT Id FROM Notifications
                                WHERE TopicHash = NEW.TopicHash
                                ORDER BY Time DESC
                                LIMIT %1
                            )
                        );
                    END;
                )").arg(numberValue));
            } else if (sourceMode == 1) {
                query.prepare(QString(R"(
                    CREATE TRIGGER NotificationsLimit
                    AFTER INSERT ON Notifications
                    WHEN (SELECT COUNT(*) FROM Notifications) > %1
                    BEGIN
                        DELETE FROM Notifications
                        WHERE Id IN (
                            SELECT Id FROM Notifications
                            ORDER BY Time ASC
                            LIMIT (SELECT COUNT(*) - %1 FROM Notifications)
                        );
                    END;
                )").arg(numberValue));
            }
            failure = failure ? true : !query.exec();
            if (failure) {std::cerr << "Failure on trigger create" << std::endl;}
        } else if (mode == 2) {
            query.prepare(QString(R"(
                CREATE TRIGGER NotificationsLimit
                AFTER INSERT ON Notifications
                BEGIN
                    DELETE FROM Notifications
                    WHERE Time < (strftime('%s', 'now') - %1);
                END;
            )").arg(recentValue));
            failure = failure ? true : !query.exec();
        } else if (mode == 3) {
            query.prepare(R"(
                CREATE TRIGGER NotificationsLimit
                AFTER INSERT ON Notifications
                BEGIN
                    DELETE FROM Notifications WHERE Id = NEW.Id;
                END;
            )");
            failure = failure ? true : !query.exec();
        }

        if (failure) {
            std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
            this->db.rollback();
        } else {
            this->db.commit();
        }
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

const std::optional<int> DataBase::getLastTimestamp(const std::string& topicHash) {
    QSqlQuery query(this->db);
    query.prepare(R"(
        SELECT Time
        FROM LastNotifications
        WHERE TopicHash = :topic_hash
        LIMIT 1
    )");
    query.bindValue(":topic_hash", QString::fromStdString(topicHash));

    if (query.exec() && query.next()) { return std::make_optional<int>(query.value(0).toInt()); }
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

void DataBase::setTlsVerificationPreference(bool preference) {
    QSqlQuery query(this->db);
    query.prepare("INSERT OR REPLACE INTO GlobalPreferences (key, value) VALUES ('TlsVerification', :preference);");
    query.bindValue(":preference", preference ? "1" : "0");
    if (!query.exec()) { std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl; }
}

void DataBase::setCAPathPreference(const std::string& preference) {
    QSqlQuery query(this->db);
    query.prepare("INSERT OR REPLACE INTO GlobalPreferences (key, value) VALUES ('CaPath', :preference);");
    query.bindValue(":preference", QString::fromStdString(preference));
    if (!query.exec()) { std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl; }
}

bool DataBase::getTlsVerificationPreference() {
    QSqlQuery query(this->db);
    query.prepare(
        "SELECT value FROM GlobalPreferences WHERE key = 'TlsVerification';"
    );
    if (query.exec() && query.next()) {
        return query.value(0).toString().toStdString() != "0";
    } else {
        return true;
    }
}

std::string DataBase::getCAPathPreference() {
    QSqlQuery query(this->db);
    query.prepare(
        "SELECT value FROM GlobalPreferences WHERE key = 'CaPath';"
    );
    if (query.exec() && query.next()) {
        return query.value(0).toString().toStdString();
    } else {
        return "";
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
