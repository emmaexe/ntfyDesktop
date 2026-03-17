#include "NtfyMessage.hpp"

#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Curl.hpp"
#include "../Util/FileManager.hpp"
#include "../Util/Util.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

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

void NtfyMessage::ViewAction::trigger_action() const {
    QDesktopServices::openUrl(this->url);
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

    std::optional<QHash<QString, QString>> extras = std::nullopt;
    if (json.contains("extras")) {
        if (!json["extras"].is_object()) {
            return std::unexpected("NtfyMessage::BroadcastAction::create: Could not parse \"extras\"");
        }

        extras = std::make_optional<QHash<QString, QString>>();
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

void NtfyMessage::BroadcastAction::trigger_action() const {
    // Nothing
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

    std::optional<QHash<QString, QString>> headers = std::nullopt;
    if (json.contains("headers")) {
        if (!json["headers"].is_object()) {
            return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"headers\"");
        }

        headers = std::make_optional<QHash<QString, QString>>();
        for (const auto& [key, value] : json["headers"].items()) {
            if (!value.is_string()) {
                return std::unexpected("NtfyMessage::HttpAction::create: Could not parse \"headers\"");
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

void NtfyMessage::HttpAction::trigger_action() const {
    auto bundle_res = Curl::Bundle::create_heap();
    if (!bundle_res) { return; }

    Curl::Bundle* bundle = *bundle_res;
    Curl::Worker& worker = bundle->worker();
    Curl::Easy& curl = worker.curl();

    curl.set_opt(CURLOPT_URL, url.toString().toStdString());

    if (this->method) {
        switch (*this->method) {
            case Method::GET:
                curl.set_opt(CURLOPT_HTTPGET, 1L);
                break;
            case Method::POST:
                curl.set_opt(CURLOPT_POST, 1L);
                break;
            case Method::PUT:
                curl.set_opt(CURLOPT_CUSTOMREQUEST, "PUT");
                break;
            case Method::DELETE:
                curl.set_opt(CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
            case Method::PATCH:
                curl.set_opt(CURLOPT_CUSTOMREQUEST, "PATCH");
                break;
            case Method::HEAD:
                curl.set_opt(CURLOPT_NOBODY, 1L);
                break;
            case Method::OPTIONS:
                curl.set_opt(CURLOPT_CUSTOMREQUEST, "OPTIONS");
                break;
            case Method::TRACE:
                curl.set_opt(CURLOPT_CUSTOMREQUEST, "TRACE");
                break;
            case Method::CONNECT:
                curl.set_opt(CURLOPT_CUSTOMREQUEST, "CONNECT");
                break;
        }
    } else {
        curl.set_opt(CURLOPT_POST, 1L);
    }

    if (this->body) {
        std::string body = this->body->toStdString();
        curl.set_opt(CURLOPT_POSTFIELDS, body);
        curl.set_opt(CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    }

    if (this->headers && !this->headers->empty()) {
        Curl::List* list = Curl::List::create_heap(bundle);

        for (const auto& [key, value]: headers->asKeyValueRange()) {
            std::string header = key.toStdString() + ": " + value.toStdString();
            list->append(header);
        }

        curl.set_opt(CURLOPT_HTTPHEADER, *list);
    }

    QObject::connect(
        &worker,
        &Curl::Worker::finished,
        bundle,
        [bundle](std::expected<void, std::string> result) {
            bundle->deleteLater();
        }
    );

    if (!bundle->start()) {
        bundle->deleteLater();
    }
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

void NtfyMessage::CopyAction::trigger_action() const {
    QApplication::clipboard()->setText(this->value);
}

std::expected<NtfyMessage::Attachment, std::string> NtfyMessage::Attachment::create(const nlohmann::json& json) {
    if (!json.is_object()) {
        return std::unexpected("NtfyMessage::Attachment::create: Incorrect input type");
    }

    if (!(json.contains("name") && json["name"].is_string())) {
        return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"name\"");
    }
    const QString name = QString::fromStdString(json["name"]);

    if (!(json.contains("url") && json["url"].is_string())) {
        return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"url\"");
    }
    const QUrl url(QString::fromStdString(json["url"]));
    if (!(url.isValid() && !url.isEmpty())) {
        return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"url\"");
    }

    std::optional<QString> type = std::nullopt;
    if (json.contains("type")) {
        if (json["type"].is_string()) {
            type = QString::fromStdString(json["type"]);
        } else {
            return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"type\"");
        }
    }

    std::optional<uint64_t> size = std::nullopt;
    if (json.contains("size")) {
        if (json["size"].is_number()) {
            try {
                size = json["size"].get<uint64_t>();
            } catch(...){
                return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"size\"");
            }
        } else {
            return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"size\"");
        }
    }

    std::optional<uint64_t> expires = std::nullopt;
    if (json.contains("expires")) {
        if (json["expires"].is_number()) {
            try {
                expires = json["expires"].get<uint64_t>();
            } catch(...){
                return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"expires\"");
            }
        } else {
            return std::unexpected("NtfyMessage::Attachment::create: Could not parse \"expires\"");
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

std::expected<QUrl, std::string> NtfyMessage::Attachment::get_temp_file() const {
    auto file_res = FileManager::instance().url_to_temp_file(this->url);
    if (file_res) {
        return *file_res;
    } else {
        return std::unexpected(std::format("NtfyMessage::Attachment::get_temp_file: {}", file_res.error()));
    }
}

void NtfyMessage::Attachment::do_user_download() const {
    QString dest_path = QFileDialog::getSaveFileName(
        nullptr,
        "Save Attachment",
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
    );
    if (dest_path.isEmpty()) { return; }

    NotificationManager::general_notification("Ntfy Desktop", "Download started: " + dest_path);

    auto file_res = FileManager::instance().url_to_temp_file(this->url);
    if (file_res) {
        QString src_path = file_res->toLocalFile();
        if (QFile::copy(src_path, dest_path)) {
            NotificationManager::general_notification("Ntfy Desktop", "Download completed: " + dest_path);
        } else {
            NotificationManager::general_notification("Ntfy Desktop", "Download failed: " + dest_path);
        }
    } else {
        NotificationManager::general_notification("Ntfy Desktop", "Download failed: " + dest_path);
    }
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

void NtfyMessage::trigger_click() const {
    if (this->click) {
        QDesktopServices::openUrl(*this->click);
    }
}

std::expected<std::optional<QUrl>, std::string> NtfyMessage::get_icon_temp_file() const {
    if (!this->icon) { return std::nullopt; }
    auto file_res = FileManager::instance().url_to_temp_file(*this->icon);
    if (file_res) {
        return *file_res;
    } else {
        return std::unexpected(std::format("NtfyMessage::get_icon_temp_file: {}", file_res.error()));
    }
}

NtfyMessage::ViewAction::ViewAction(
    QString label,
    QUrl url,
    std::optional<bool> clear
):
    label(std::move(label)),
    url(std::move(url)),
    clear(clear) {}

NtfyMessage::BroadcastAction::BroadcastAction(
    QString label,
    QString intent,
    std::optional<QHash<QString, QString>> extras,
    std::optional<bool> clear
):
    label(std::move(label)),
    intent(std::move(intent)),
    extras(std::move(extras)),
    clear(clear) {}

NtfyMessage::HttpAction::HttpAction(
    QString label,
    QUrl url,
    std::optional<Method> method,
    std::optional<QHash<QString, QString>> headers,
    std::optional<QString> body,
    std::optional<bool> clear
):
    label(std::move(label)),
    url(std::move(url)),
    method(method),
    headers(std::move(headers)),
    body(std::move(body)),
    clear(clear) {}

NtfyMessage::CopyAction::CopyAction(
    QString label,
    QString value,
    std::optional<bool> clear
):
    label(std::move(label)),
    value(std::move(value)),
    clear(clear) {}

NtfyMessage::Attachment::Attachment(
    QString name,
    QUrl url,
    std::optional<QString> type,
    std::optional<uint64_t> size,
    std::optional<uint64_t> expires
):
    name(std::move(name)),
    url(std::move(url)),
    type(std::move(type)),
    size(size),
    expires(expires) {}

NtfyMessage::NtfyMessage(
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
):
    json_str(std::move(json_str)),
    id(std::move(id)),
    time(time),
    expires(expires),
    event(event),
    topic(std::move(topic)),
    sequence_id(std::move(sequence_id)),
    message(std::move(message)),
    title(std::move(title)),
    tags(std::move(tags)),
    priority(priority),
    click(std::move(click)),
    actions(std::move(actions)),
    attachment(std::move(attachment)),
    icon(std::move(icon)),
    content_type(content_type) {}
