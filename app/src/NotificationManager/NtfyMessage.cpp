#include "NtfyMessage.hpp"

#include "../Util/Util.hpp"

#include <emojicpp/emoji.hpp>

std::expected<NtfyMessage, std::string> NtfyMessage::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::create: Input json is not an object");
    }

    const QString json_str = QString::fromStdString(json.dump());

    if (!(json.contains("id") && json["id"].is_string())) {
        return std::unexpected("NtfyMessage::create: Failed to parse \"id\"");
    }
    const QString id = QString::fromStdString(json["id"]);

    if (!(json.contains("time") && json["time"].is_number())) {
        return std::unexpected("NtfyMessage::create: Failed to parse \"time\"");
    }
    const uint64_t time = json["time"];

    if (!(json.contains("expires") && json["expires"].is_number())) {
        return std::unexpected("NtfyMessage::create: Failed to parse \"expires\"");
    }
    const uint64_t expires = json["expires"];

    if (!json.contains("event")) {
        return std::unexpected("NtfyMessage::create: Failed to parse \"event\"");
    }
    auto event_res = Event::from_json(json["event"]);
    if (!event_res) {
        return std::unexpected(std::format("NtfyMessage::create: {}", event_res.error()));
    }
    const Event::Value event = *event_res;

    if (!(json.contains("topic") && json["topic"].is_string())) {
        return std::unexpected("NtfyMessage::create: Failed to parse \"topic\"");
    }
    const QString topic = QString::fromStdString(json["topic"]);

    std::optional<QString> sequence_id = std::nullopt;
    if (json.contains("sequence_id") && json["sequence_id"].is_string()) {
        sequence_id = QString::fromStdString(json["sequence_id"]);
    }

    std::optional<QString> message = std::nullopt;
    if (json.contains("message") && json["message"].is_string()) {
        message = QString::fromStdString(json["message"]);
    }

    std::optional<QString> title = std::nullopt;
    if (json.contains("title") && json["title"].is_string()) {
        title = QString::fromStdString(json["title"]);
    }

    std::optional<std::vector<QString>> tags = std::nullopt;
    if (json.contains("tags") && json["tags"].is_array()) {
        tags = std::make_optional<std::vector<QString>>();

        for (auto& tag: json["tags"]) {
            if (tag.is_string()) {
                std::string raw_tag = ":" + tag.get<std::string>() + ":";
                std::string parsed_tag = emojicpp::emoji::parse(raw_tag);
                if (parsed_tag == raw_tag) {
                    tags->push_back(QString::fromStdString(tag));
                } else {
                    tags->push_back(QString::fromStdString(parsed_tag));
                }
            } else {
                return std::unexpected("NtfyMessage::create: Failed to parse \"tags\"");
            }
        }
    }

    std::optional<Priority::Value> priority = std::nullopt;
    if (json.contains("priority")) {
        const auto priority_res = Priority::from_json(json["priority"]);
        if (priority_res) {
            priority = *priority_res;
        }
    }

    std::optional<QUrl> click = std::nullopt;
    if (json.contains("click") && json["click"].is_string()) {
        QUrl url(QString::fromStdString(json["click"].get<std::string>()));
        if (url.isValid() && !url.isEmpty()) {
            click = url;
        } else {
            return std::unexpected("NtfyMessage::create: Failed to parse \"click\"");
        }
    }

    std::optional<std::vector<std::variant<ViewAction, BroadcastAction, HttpAction, CopyAction>>> actions = std::nullopt;
    if (json.contains("actions")) {
        if (!json["actions"].is_array()) {
            return std::unexpected("NtfyMessage::create: Failed to parse \"actions\"");
        }

        actions = std::make_optional<std::vector<std::variant<ViewAction, BroadcastAction, HttpAction, CopyAction>>>();

        for (auto& action: json["actions"]) {
            if (!(action.is_object() && action.contains("action") && action["action"].is_string())) {
                return std::unexpected("NtfyMessage::create: Failed to parse \"actions\"");
            }

            if (action["action"] == "view") {
                const auto action_res = ViewAction::create(action);
                if (!action_res) {
                    return std::unexpected(std::format("NtfyMessage::create: {}", action_res.error()));
                }
                actions->push_back(std::move(*action_res));
            } else if (action["action"] == "broadcast") {
                const auto action_res = BroadcastAction::create(action);
                if (!action_res) {
                    return std::unexpected(std::format("NtfyMessage::create: {}", action_res.error()));
                }
                actions->push_back(std::move(*action_res));
            } else if (action["action"] == "http") {
                const auto action_res = HttpAction::create(action);
                if (!action_res) {
                    return std::unexpected(std::format("NtfyMessage::create: {}", action_res.error()));
                }
                actions->push_back(std::move(*action_res));
            } else if (action["action"] == "copy") {
                const auto action_res = CopyAction::create(action);
                if (!action_res) {
                    return std::unexpected(std::format("NtfyMessage::create: {}", action_res.error()));
                }
                actions->push_back(std::move(*action_res));
            } else {
                return std::unexpected("NtfyMessage::create: Failed to parse \"actions\"");
            }
        }
    }

    std::optional<Attachment> attachment = std::nullopt;
    if (json.contains("attachment")) {
        const auto attachment_res = Attachment::create(json["attachment"]);
        if (attachment_res) {
            attachment.emplace(std::move(*attachment_res));
        }
    }

    std::optional<QUrl> icon = std::nullopt;
    if (json.contains("icon") && json["icon"].is_string()) {
        QUrl url(QString::fromStdString(json["icon"].get<std::string>()));
        if (url.isValid() && !url.isEmpty()) {
            icon = url;
        } else {
            return std::unexpected("NtfyMessage::create: Failed to parse \"icon\"");
        }
    }

    std::optional<ContentType::Value> content_type = std::nullopt;
    if (json.contains("content_type")) {
        const auto content_type_res = ContentType::from_json(json["content_type"]);
        if (content_type_res) {
            content_type = *content_type_res;
        }
    }

    return NtfyMessage(
        json_str,
        id,
        time,
        expires,
        event,
        topic,
        sequence_id,
        message,
        title,
        tags,
        priority,
        click,
        actions,
        attachment,
        icon,
        content_type
    );
}

std::expected<NtfyMessage::Event::Value, std::string> NtfyMessage::Event::from_json(const nlohmann::json& json) {
    if (!json.is_string()) {
        return std::unexpected("NtfyMessage::Event::from_json: Incorrect input type");
    }

    std::string value = json.get<std::string>();
    Util::Strings::toUpper(value);

    if (value == "OPEN") {
        return Event::OPEN;
    } else if (value == "KEEPALIVE") {
        return Event::KEEPALIVE;
    } else if (value == "MESSAGE") {
        return Event::MESSAGE;
    } else if (value == "MESSAGE_DELETE") {
        return Event::MESSAGE_DELETE;
    } else if (value == "MESSAGE_CLEAR") {
        return Event::MESSAGE_CLEAR;
    } else if (value == "POLL_REQUEST") {
        return Event::POLL_REQUEST;
    } else {
        return std::unexpected("NtfyMessage::Event::from_json: Input is not valid");
    }
}

std::expected<NtfyMessage::Priority::Value, std::string> NtfyMessage::Priority::from_json(const nlohmann::json& json) {
    if (!json.is_number()) {
        return std::unexpected("NtfyMessage::Priority::from_json: Incorrect input type");
    }

    int value;
    try {
        value = json.get<int>();
    } catch(...) {
        return std::unexpected("NtfyMessage::Priority::from_json: Input is not valid");
    }

    if (!(1 <= value && value <= 5)) {
        return std::unexpected("NtfyMessage::Priority::from_json: Input is not valid");
    }

    return static_cast<Priority::Value>(value);
}

std::expected<NtfyMessage::ViewAction, std::string> NtfyMessage::ViewAction::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::ViewAction::create: Incorrect input type");
    }

    if (!(json.contains("label") && json["label"].is_string())) {
        return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"label\"");
    }
    const QString label = QString::fromStdString(json["label"]);

    if (!(json.contains("url") && json["url"].is_string())) {
        return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"url\"");
    }
    const QUrl url(QString::fromStdString(json["url"]));
    if (!(url.isValid() && !url.isEmpty())) {
        return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"url\"");
    }

    std::optional<bool> clear = std::nullopt;
    if (json.contains("clear")) {
        if (json["clear"].is_boolean()) {
            clear = json["clear"].get<bool>();
        } else {
            return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"clear\"");
        }
    }

    return NtfyMessage::ViewAction(
        label,
        url,
        clear
    );
}

std::expected<NtfyMessage::BroadcastAction, std::string> NtfyMessage::BroadcastAction::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::BroadcastAction::create: Incorrect input type");
    }

    if (!(json.contains("label") && json["label"].is_string())) {
        return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"label\"");
    }
    const QString label = QString::fromStdString(json["label"]);

    if (!(json.contains("intent") && json["intent"].is_string())) {
        return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"intent\"");
    }
    const QString intent = QString::fromStdString(json["intent"]);

    std::optional<std::unordered_map<QString, QString>> extras = std::nullopt;
    if (json.contains("extras")) {
        if (!json["extras"].is_object()) {
            return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"extras\"");
        }

        extras = std::make_optional<std::unordered_map<QString, QString>>();
        for (const auto& [key, value] : json["extras"].items()) {
            if (!value.is_string()) {
                return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"extras\"");
            }

            extras->emplace(
                QString::fromStdString(key),
                QString::fromStdString(value.get<std::string>())
            );
        }
    }

    std::optional<bool> clear = std::nullopt;
    if (json.contains("clear")) {
        if (json["clear"].is_boolean()) {
            clear = json["clear"].get<bool>();
        } else {
            return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"clear\"");
        }
    }

    return NtfyMessage::BroadcastAction(
        label,
        intent,
        extras,
        clear
    );
}

std::expected<NtfyMessage::HttpAction, std::string> NtfyMessage::HttpAction::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::HttpAction::create: Incorrect input type");
    }

    if (!(json.contains("label") && json["label"].is_string())) {
        return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"label\"");
    }
    const QString label = QString::fromStdString(json["label"]);

    if (!(json.contains("url") && json["url"].is_string())) {
        return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"url\"");
    }
    const QUrl url(QString::fromStdString(json["url"]));
    if (!(url.isValid() && !url.isEmpty())) {
        return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"url\"");
    }

    std::optional<Method> method = std::nullopt;
    if (json.contains("method")) {
        if (!json["method"].is_string()) {
            return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"method\"");
        }

        std::string method_str = json["method"];
        Util::Strings::toUpper(method_str);

        if (method_str == "GET") {
            method = Method::GET;
        } else if (method_str == "HEAD") {
            method = Method::HEAD;
        } else if (method_str == "POST") {
            method = Method::POST;
        } else if (method_str == "PUT") {
            method = Method::PUT;
        } else if (method_str == "DELETE") {
            method = Method::DELETE;
        } else if (method_str == "CONNECT") {
            method = Method::CONNECT;
        } else if (method_str == "OPTIONS") {
            method = Method::OPTIONS;
        } else if (method_str == "TRACE") {
            method = Method::TRACE;
        } else if (method_str == "PATCH") {
            method = Method::PATCH;
        } else {
            return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"method\"");
        }
    }

    std::optional<std::unordered_map<QString, QString>> headers = std::nullopt;
    if (json.contains("headers")) {
        if (!json["headers"].is_object()) {
            return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"headers\"");
        }

        headers = std::make_optional<std::unordered_map<QString, QString>>();
        for (const auto& [key, value] : json["headers"].items()) {
            if (!value.is_string()) {
                return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"headers\"");
            }

            headers->emplace(
                QString::fromStdString(key),
                QString::fromStdString(value.get<std::string>())
            );
        }
    }

    std::optional<QString> body = std::nullopt;
    if (json.contains("body")) {
        if (json["body"].is_string()) {
            body = QString::fromStdString(json["body"]);
        } else {
            return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"body\"");
        }
    }

    std::optional<bool> clear = std::nullopt;
    if (json.contains("clear")) {
        if (json["clear"].is_boolean()) {
            clear = json["clear"].get<bool>();
        } else {
            return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"clear\"");
        }
    }

    return NtfyMessage::HttpAction(
        label,
        url,
        method,
        headers,
        body,
        clear
    );
}

std::expected<NtfyMessage::CopyAction, std::string> NtfyMessage::CopyAction::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::CopyAction::create: Incorrect input type");
    }

    if (!(json.contains("label") && json["label"].is_string())) {
        return std::unexpected("NtfyMessage::CopyAction::create: Could not parse \"label\"");
    }
    const QString label = QString::fromStdString(json["label"]);

    if (!(json.contains("value") && json["value"].is_string())) {
        return std::unexpected("NtfyMessage::CopyAction::create: Could not parse \"value\"");
    }
    const QString value = QString::fromStdString(json["value"]);

    std::optional<bool> clear = std::nullopt;
    if (json.contains("clear")) {
        if (json["clear"].is_boolean()) {
            clear = json["clear"].get<bool>();
        } else {
            return std::unexpected("NtfyMessage::CopyAction::create: Could not parse \"clear\"");
        }
    }

    return NtfyMessage::CopyAction(
        label,
        value,
        clear
    );
}

std::expected<NtfyMessage::Attachment, std::string> NtfyMessage::Attachment::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::Attachment::create: Incorrect input type");
    }

    if (!(json.contains("name") && json["name"].is_string())) {
        return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"name\"");
    }
    const QString name = QString::fromStdString(json["name"]);

    if (!(json.contains("url") && json["url"].is_string())) {
        return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"url\"");
    }
    const QUrl url(QString::fromStdString(json["url"]));
    if (!(url.isValid() && !url.isEmpty())) {
        return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"url\"");
    }

    std::optional<QString> type = std::nullopt;
    if (json.contains("type")) {
        if (json["type"].is_string()) {
            type = QString::fromStdString(json["type"]);
        } else {
            return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"type\"");
        }
    }

    std::optional<uint64_t> size = std::nullopt;
    if (json.contains("size")) {
        if (json["size"].is_number()) {
            try {
                size = json["size"].get<uint64_t>();
            } catch(...){
                return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"size\"");
            }
        } else {
            return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"size\"");
        }
    }

    std::optional<uint64_t> expires = std::nullopt;
    if (json.contains("expires")) {
        if (json["expires"].is_number()) {
            try {
                expires = json["expires"].get<uint64_t>();
            } catch(...){
                return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"expires\"");
            }
        } else {
            return std::unexpected("NtfyMessage::ViewAction::create: Could not parse \"expires\"");
        }
    }

    return NtfyMessage::Attachment(
        name,
        url,
        type,
        size,
        expires
    );

}

std::expected<NtfyMessage::ContentType::Value, std::string> NtfyMessage::ContentType::from_json(const nlohmann::json& json) {
    if (!json.is_string()) {
        return std::unexpected("NtfyMessage::ContentType::from_json: Incorrect input type");
    }

    std::string value = json.get<std::string>();
    Util::Strings::toUpper(value);

    if (value == "TEXT_PLAIN") {
        return ContentType::TEXT_PLAIN;
    } else if (value == "TEXT_MARKDOWN") {
        return ContentType::TEXT_MARKDOWN;
    } else {
        return std::unexpected("NtfyMessage::ContentType::from_json: Input is not valid");
    }
}

NtfyMessage::ViewAction::ViewAction(
    const QString label,
    const QUrl url,
    const std::optional<bool> clear
):
    label(label),
    url(url),
    clear(clear) {}

NtfyMessage::BroadcastAction::BroadcastAction(
    const QString label,
    const QString intent,
    const std::optional<std::unordered_map<QString, QString>> extras,
    const std::optional<bool> clear
):
    label(label),
    intent(intent),
    extras(extras),
    clear(clear) {}

NtfyMessage::HttpAction::HttpAction(
    const QString label,
    const QUrl url,
    const std::optional<Method> method,
    const std::optional<std::unordered_map<QString, QString>> headers,
    const std::optional<QString> body,
    const std::optional<bool> clear
):
    label(label),
    url(url),
    method(method),
    headers(headers),
    body(body),
    clear(clear) {}

NtfyMessage::CopyAction::CopyAction(
    const QString label,
    const QString value,
    const std::optional<bool> clear
):
    label(label),
    value(value),
    clear(clear) {}

NtfyMessage::Attachment::Attachment(
    const QString name,
    const QUrl url,
    const std::optional<QString> type,
    const std::optional<uint64_t> size,
    const std::optional<uint64_t> expires
):
    name(name),
    url(url),
    type(type),
    size(size),
    expires(expires) {}

NtfyMessage::NtfyMessage(
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
):
    json_str(json_str),
    id(id),
    time(time),
    expires(expires),
    event(event),
    topic(topic),
    sequence_id(sequence_id),
    message(message),
    title(title),
    tags(tags),
    priority(priority),
    click(click),
    actions(actions),
    attachment(attachment),
    icon(icon),
    content_type(content_type) {}
