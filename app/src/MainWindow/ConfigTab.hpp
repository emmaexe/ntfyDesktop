#pragma once

#include "../DataBase/DataBase.hpp"

#include <QThread>
#include <QTimer>
#include <QWidget>
#include <atomic>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ConfigTab;
}
QT_END_NAMESPACE

class ConfigTab: public QWidget {
        Q_OBJECT
    public:
        ConfigTab(std::string name = "", std::string domain = "", std::string topic = "", AuthConfig authConfig = AuthConfig(), bool secure = true, QWidget* parent = nullptr);
        ~ConfigTab();
        std::string getName();
        std::string getDomain();
        std::string getTopic();
        bool getSecure();
        AuthConfig getAuth();
        void clearInvisible();
    public slots:
        void testButton();
        void testResults(const bool& result);
        void authTypeChanged(int index);
    private:
        Ui::ConfigTab* ui;
        QTimer* testLabelTimer;
        std::string lastTestMessage;
};

class ConnectionTester: public QObject {
        Q_OBJECT
    public:
        ConnectionTester(const std::string& name, const std::string& domain, const std::string& topic, const AuthConfig& authConfig, const bool& secure, std::string& testMessage);
    public slots:
        void runTest();
    signals:
        void testFinished(const bool& success);
    private:
        std::string& testMessage;
        std::string name, domain, topic;
        AuthConfig authConfig;
        bool secure;
        std::atomic<bool> testSuccessful;
        static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
};
