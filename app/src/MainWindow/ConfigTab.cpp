#include "ConfigTab.hpp"
#include "ui_ConfigTab.h"

ConfigTab::ConfigTab(std::string name, std::string server, std::string topic, QWidget* parent): QWidget(parent), ui(new Ui::ConfigTab) {
    this->ui->setupUi(this);
    this->ui->nameLineEdit->setText(name.c_str());
    this->ui->serverLineEdit->setText(server.c_str());
    this->ui->topicLineEdit->setText(topic.c_str());
}

ConfigTab::~ConfigTab() {
    delete ui;
}

std::string ConfigTab::getName() {
    return this->ui->nameLineEdit->text().toStdString();
}

std::string ConfigTab::getServer() {
    return this->ui->serverLineEdit->text().toStdString();
}

std::string ConfigTab::getTopic() {
    return this->ui->topicLineEdit->text().toStdString();
}

