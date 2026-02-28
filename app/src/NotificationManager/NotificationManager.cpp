#include "NotificationManager.hpp"

#include "../Config/Config.hpp"
#include "../Util/FileManager.hpp"
#include "../Util/Logging.hpp"
#include "ntfyDesktop.hpp"

#include <KNotification>
#include <QDesktopServices>

namespace NotificationManager {
    void ntfy_notification(const NtfyMessage message) {
        if (message.event != NtfyMessage::Event::MESSAGE) { return; }
        KNotification* notification = new KNotification("ntfy");

        notification->setTitle(message.title.value_or("Ntfy Desktop - " + message.topic));

        QString text = message.message.value_or("No message attached.");
        if (message.content_type == NtfyMessage::ContentType::TEXT_MARKDOWN) {
            // ~todo: Render markdown text to Qt's Text.StyledText
        }
        if (message.tags && message.tags->size() > 0) {
            text += " Tags: ";
            for (const QString& tag: *message.tags) {
                text += tag + " ";
            }
        }
        notification->setText(text);

        notification->setIconName("moe.emmaexe.ntfyDesktop");
        if (message.icon && ND_BUILD_TYPE != "Flatpak") {
            auto icon_file_res = message.get_icon_temp_file();
            if (icon_file_res) {
                QString file_path = (*icon_file_res)->toLocalFile();

                QPixmap pixmap;
                if (pixmap.load(file_path)) {
                    notification->setPixmap(pixmap);
                }
            }
        }

        if (message.priority) {
            if (message.priority == NtfyMessage::Priority::HIGHEST) {
                notification->setUrgency(KNotification::Urgency::CriticalUrgency);
            } else if (message.priority == NtfyMessage::Priority::HIGH) {
                notification->setUrgency(KNotification::Urgency::HighUrgency);
            } else if (message.priority == NtfyMessage::Priority::NORMAL) {
                notification->setUrgency(KNotification::Urgency::NormalUrgency);
            } else if (message.priority == NtfyMessage::Priority::LOW || message.priority == NtfyMessage::Priority::LOWEST) {
                notification->setUrgency(KNotification::Urgency::LowUrgency);
            }
        }

        if (message.click) {
            KNotificationAction* action = notification->addDefaultAction("Open URL");
            KNotificationAction::connect(
                action,
                &KNotificationAction::activated,
                [message]() {
                    message.trigger_click();
                }
            );
        }

        if (message.actions && message.actions->size() > 0) {
            for (const auto& action: *message.actions) {
                if (std::holds_alternative<NtfyMessage::BroadcastAction>(action)) { continue; }

                std::visit([&notification](auto&& action) {
                    KNotificationAction* knaction = notification->addAction(action.label);
                    KNotificationAction::connect(
                        knaction,
                        &KNotificationAction::activated,
                        [action]() {
                            action.trigger_action();
                        }
                    );
                }, action);
            }
        }

        if (message.attachment) {
            KNotificationAction* action = notification->addAction("Download Attachment");
            KNotificationAction::connect(
                action,
                &KNotificationAction::activated,
                [attachment = *message.attachment]() {
                    attachment.do_user_download();
                }
            );

            if (ND_BUILD_TYPE != "Flatpak") {
                auto file_res = message.attachment->get_temp_file();
                if (file_res) {
                    notification->setUrls({ *file_res });
                }
            }
        }

        notification->sendEvent();
    }

    void general_notification(const QString title, const QString message) {
        KNotification* notification = new KNotification("general");
        notification->setUrgency(KNotification::Urgency::NormalUrgency);
        notification->setTitle(title);
        notification->setText(message);
        notification->setIconName("moe.emmaexe.ntfyDesktop");
        notification->sendEvent();
    }

    void startup_notification() {
        if (Config::data()["preferences"].is_object() && Config::data()["preferences"]["notifications"].is_object() && Config::data()["preferences"]["notifications"]["startup"].is_boolean() && Config::data()["preferences"]["notifications"]["startup"]) {
            KNotification* notification = new KNotification("startup");
            notification->setTitle("Ntfy Desktop");
            notification->setText("Ntfy Desktop is running in the background.");
            notification->setUrgency(KNotification::Urgency::LowUrgency);
            notification->setIconName("moe.emmaexe.ntfyDesktop");
            notification->sendEvent();
        }
    }

    void error_notification(const QString title, const QString message) {
        if (Config::data()["preferences"].is_object() && Config::data()["preferences"]["notifications"].is_object() && Config::data()["preferences"]["notifications"]["error"].is_boolean() && Config::data()["preferences"]["notifications"]["error"]) {
            KNotification* notification = new KNotification("error");
            notification->setTitle(title);
            notification->setText(message);
            notification->setUrgency(KNotification::Urgency::HighUrgency);
            notification->setIconName("moe.emmaexe.ntfyDesktop");
            notification->sendEvent();
        }
    }
}
