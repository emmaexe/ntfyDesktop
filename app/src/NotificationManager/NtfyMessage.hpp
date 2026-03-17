#pragma once

#include <nlohmann/json.hpp>

#include <QHash>
#include <QString>
#include <QObject>
#include <QUrl>

#include <cstdint>
#include <expected>
#include <optional>
#include <variant>
#include <vector>

class NtfyMessage {
    public:
        static std::expected<NtfyMessage, std::string> create(const nlohmann::json& json);

        struct Event {
            enum Value { OPEN, KEEPALIVE, MESSAGE, MESSAGE_DELETE, MESSAGE_CLEAR, POLL_REQUEST };

            static std::expected<Value, std::string> from_json(const nlohmann::json& json);
        };
        struct Priority {
            enum Value { LOWEST = 1, LOW = 2, NORMAL = 3, HIGH = 4, HIGHEST = 5 };

            static std::expected<Value, std::string> from_json(const nlohmann::json& json);
        };
        class ViewAction {
            public:
                static std::expected<ViewAction, std::string> create(const nlohmann::json& json);

                const QString label;
                const QUrl url;
                const std::optional<bool> clear;

                void trigger_action() const;
            private:
                ViewAction(
                    QString label,
                    QUrl url,
                    std::optional<bool> clear
                );
        };
        class BroadcastAction {
            public:
                static std::expected<BroadcastAction, std::string> create(const nlohmann::json& json);

                const QString label;
                const QString intent;
                const std::optional<QHash<QString, QString>> extras;
                const std::optional<bool> clear;

                void trigger_action() const;
            private:
                BroadcastAction(
                    QString label,
                    QString intent,
                    std::optional<QHash<QString, QString>> extras,
                    std::optional<bool> clear
                );
        };
        class HttpAction {
            public:
                static std::expected<HttpAction, std::string> create(const nlohmann::json& json);

                enum class Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

                const QString label;
                const QUrl url;
                const std::optional<Method> method;
                const std::optional<QHash<QString, QString>> headers;
                const std::optional<QString> body;
                const std::optional<bool> clear;

                void trigger_action() const;
            private:
                HttpAction(
                    QString label,
                    QUrl url,
                    std::optional<Method> method,
                    std::optional<QHash<QString, QString>> headers,
                    std::optional<QString> body,
                    std::optional<bool> clear
                );
        };
        class CopyAction {
            public:
                static std::expected<CopyAction, std::string> create(const nlohmann::json& json);

                const QString label;
                const QString value;
                const std::optional<bool> clear;

                void trigger_action() const;
            private:
                CopyAction(
                    QString label,
                    QString value,
                    std::optional<bool> clear
                );
        };
        class Attachment {
            public:
                static std::expected<Attachment, std::string> create(const nlohmann::json& json);

                const QString name;
                const QUrl url;
                const std::optional<QString> type;
                const std::optional<uint64_t> size;
                const std::optional<uint64_t> expires;

                std::expected<QUrl, std::string> get_temp_file() const;
                void do_user_download() const;
            private:
                Attachment(
                    QString name,
                    QUrl url,
                    std::optional<QString> type,
                    std::optional<uint64_t> size,
                    std::optional<uint64_t> expires
                );
        };
        struct ContentType {
            enum Value { TEXT_PLAIN, TEXT_MARKDOWN };

            static std::expected<Value, std::string> from_json(const nlohmann::json& json);
        };

        const QString json_str;

        const QString id;
        const uint64_t time;
        const uint64_t expires;
        const Event::Value event;
        const QString topic;
        const std::optional<QString> sequence_id;
        const std::optional<QString> message;
        const std::optional<QString> title;
        const std::optional<std::vector<QString>> tags;
        const std::optional<Priority::Value> priority;
        const std::optional<QUrl> click;
        const std::optional<std::vector<std::variant<ViewAction, BroadcastAction, HttpAction, CopyAction>>> actions;
        const std::optional<Attachment> attachment;

        const std::optional<QUrl> icon;
        const std::optional<ContentType::Value> content_type;

        void trigger_click() const;
        std::expected<std::optional<QUrl>, std::string> get_icon_temp_file() const;
    private:
        NtfyMessage(
            QString json_str,
            QString id,
            uint64_t time,
            uint64_t expires,
            Event::Value event,
            QString topic,
            std::optional<QString> sequence_id,
            std::optional<QString> message,
            std::optional<QString> title,
            std::optional<std::vector<QString>> tags,
            std::optional<Priority::Value> priority,
            std::optional<QUrl> click,
            std::optional<std::vector<std::variant<ViewAction, BroadcastAction, HttpAction, CopyAction>>> actions,
            std::optional<Attachment> attachment,
            std::optional<QUrl> icon,
            std::optional<ContentType::Value> content_type
        );
};
