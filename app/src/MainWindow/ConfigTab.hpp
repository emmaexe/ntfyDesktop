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
        ConfigTab(std::string name, std::string server, std::string topic, QWidget* parent = nullptr);
        ~ConfigTab();
        std::string getName();
        std::string getServer();
        std::string getTopic();
    private:
        Ui::ConfigTab* ui;
};
