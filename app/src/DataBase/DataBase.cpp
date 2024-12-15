#include "DataBase.hpp"

#include "ntfyDesktop.hpp"

#include <QUuid>
#include <QStandardPaths>
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
        )")
    ) {
        this->db.commit();
    } else {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
        this->db.rollback();
    }
}

DataBase::~DataBase() {
    if (this->db.isOpen()) { this->db.close(); }
    this->db = QSqlDatabase(); // Qt logs unhappy messages when there is a QSqlDatabase object still connected to a db that is being removed.
    QSqlDatabase::removeDatabase(QString::fromStdString(this->connectionName));
}

void DataBase::setAuth(const std::string& topicHash, const AuthConfig& authConfig) {
    QSqlQuery query(this->db);
    if (authConfig.type == AuthType::NONE) {
        query.prepare(R"(
            INSERT OR REPLACE INTO Auth (TopicHash, AuthType)
            VALUES (?, ?)
        )");
        query.addBindValue(QString::fromStdString(topicHash));
        query.addBindValue(authConfig.type);
    } else if (authConfig.type == AuthType::USERNAME_PASSWORD) {
        query.prepare(R"(
            INSERT OR REPLACE INTO Auth (TopicHash, AuthType, Username, Password)
            VALUES (?, ?, ?, ?)
        )");
        query.addBindValue(QString::fromStdString(topicHash));
        query.addBindValue(authConfig.type);
        query.addBindValue(QString::fromStdString(authConfig.username));
        query.addBindValue(QString::fromStdString(authConfig.password));
    } else if (authConfig.type == AuthType::TOKEN) {
        query.prepare(R"(
            INSERT OR REPLACE INTO Auth (TopicHash, AuthType, Token)
            VALUES (?, ?, ?)
        )");
        query.addBindValue(QString::fromStdString(topicHash));
        query.addBindValue(authConfig.type);
        query.addBindValue(QString::fromStdString(authConfig.token));
    }
    if (!query.exec()) {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
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

void DataBase::executeQuery(const std::string& queryText) {
    QSqlQuery query(QString::fromStdString(queryText), this->db);
    if (!query.exec()) {
        std::cerr << "DataBase query failed: " << query.lastError().text().toStdString() << std::endl;
    }
}

const std::string DataBase::getDBPath() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString(); }

const std::string DataBase::getDBFile() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + std::string("/db.sqlite3"); }
