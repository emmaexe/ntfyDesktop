#pragma once

#include <QObject>
#include <QString>
#include <optional>
#include <mutex>

class SingleInstanceManager: public QObject {
        Q_OBJECT
        Q_DISABLE_COPY(SingleInstanceManager)
        Q_CLASSINFO("D-Bus Interface", "moe.emmaexe.ntfyDesktop.SingleInstanceManager")
    public:
        static SingleInstanceManager* get();
        void init(std::optional<QString> url);
    public slots:
        void newInstance(bool has_value, QString url);
    signals:
        void new_instance(std::optional<QString> url);
    private:
        SingleInstanceManager(QObject* parent = nullptr);
        std::once_flag init_flag;
};
