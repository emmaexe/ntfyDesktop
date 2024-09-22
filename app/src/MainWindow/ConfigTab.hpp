#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ConfigTab;
}
QT_END_NAMESPACE

class ConfigTab: public QWidget {
        Q_OBJECT
    public:
        ConfigTab(std::string name = "", std::string server = "", std::string topic = "", bool secure = true, QWidget* parent = nullptr);
        ~ConfigTab();
        std::string getName();
        std::string getDomain();
        std::string getTopic();
        bool getSecure();
    private:
        Ui::ConfigTab* ui;
};
