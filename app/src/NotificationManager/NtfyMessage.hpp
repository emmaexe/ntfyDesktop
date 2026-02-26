#pragma once

#include <nlohmann/json.hpp>

#include <QString>
#include <QObject>
#include <QUrl>

#include <cstdint>
#include <expected>
#include <optional>
#include <unordered_map>
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
            private:
                ViewAction(
                    const QString label,
                    const QUrl url,
                    const std::optional<bool> clear
                );
        };
        class BroadcastAction {
            public:
                static std::expected<BroadcastAction, std::string> create(const nlohmann::json& json);

                const QString label;
                const QString intent;
                const std::optional<std::unordered_map<QString, QString>> extras;
                const std::optional<bool> clear;
            private:
                BroadcastAction(
                    const QString label,
                    const QString intent,
                    const std::optional<std::unordered_map<QString, QString>> extras,
                    const std::optional<bool> clear
                );
        };
        class HttpAction {
            public:
                static std::expected<HttpAction, std::string> create(const nlohmann::json& json);

                enum class Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

                const QString label;
                const QUrl url;
                const std::optional<Method> method;
                const std::optional<std::unordered_map<QString, QString>> headers;
                const std::optional<QString> body;
                const std::optional<bool> clear;
            private:
                HttpAction(
                    const QString label,
                    const QUrl url,
                    const std::optional<Method> method,
                    const std::optional<std::unordered_map<QString, QString>> headers,
                    const std::optional<QString> body,
                    const std::optional<bool> clear
                );
        };
        class CopyAction {
            public:
                static std::expected<CopyAction, std::string> create(const nlohmann::json& json);

                const QString label;
                const QString value;
                const std::optional<bool> clear;
            private:
                CopyAction(
                    const QString label,
                    const QString value,
                    const std::optional<bool> clear
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
            private:
                Attachment(
                    const QString name,
                    const QUrl url,
                    const std::optional<QString> type,
                    const std::optional<uint64_t> size,
                    const std::optional<uint64_t> expires
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
    private:
        NtfyMessage(
            const QString json_str,
            const QString id,
            const uint64_t time,
            const uint64_t expires,
            const Event::Value event,
            const QString topic,
            const std::optional<QString> sequence_id,
            const std::optional<QString> message,
            const std::optional<QString> title,
            const std::optional<std::vector<QString>> tags,
            const std::optional<Priority::Value> priority,
            const std::optional<QUrl> click,
            const std::optional<std::vector<std::variant<ViewAction, BroadcastAction, HttpAction, CopyAction>>> actions,
            const std::optional<Attachment> attachment,
            const std::optional<QUrl> icon,
            const std::optional<ContentType::Value> content_type
        );
};
