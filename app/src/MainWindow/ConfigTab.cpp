#include "ConfigTab.hpp"

#include "ui_ConfigTab.h"

ConfigTab::ConfigTab(std::string name, std::string server, std::string topic, bool secure, QWidget* parent): QWidget(parent), ui(new Ui::ConfigTab) {
    this->ui->setupUi(this);
    this->ui->nameLineEdit->setText(QString::fromStdString(name));
    this->ui->domainLineEdit->setText(QString::fromStdString(server));
    this->ui->topicLineEdit->setText(QString::fromStdString(topic));
    this->ui->secureCheckBox->setChecked(secure);
}

ConfigTab::~ConfigTab() { delete ui; }

std::string ConfigTab::getName() { return this->ui->nameLineEdit->text().toStdString(); }

std::string ConfigTab::getDomain() { return this->ui->domainLineEdit->text().toStdString(); }

std::string ConfigTab::getTopic() { return this->ui->topicLineEdit->text().toStdString(); }

bool ConfigTab::getSecure() { return this->ui->secureCheckBox->isChecked(); }
