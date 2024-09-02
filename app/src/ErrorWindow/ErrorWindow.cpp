#include "ErrorWindow.hpp"
#include "ui_ErrorWindow.h"

#include "ntfyDesktop.hpp"

ErrorWindow::ErrorWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::ErrorWindow) {
    this->ui->setupUi(this);
    this->ui->ErrorText->setText(QLabel::tr(Config::getError().c_str()));
    QObject::connect(this->ui->ResetButton, &QPushButton::clicked, this, &ErrorWindow::resetConfig);
}

ErrorWindow::~ErrorWindow() {
    delete ui;
}

void ErrorWindow::resetConfig() {

    Config::data()["version"] = ND_VERSION;
    Config::data()["sources"] = nlohmann::json::array();
    Config::write();
    this->ui->ErrorText->setText(QLabel::tr("The configuration was reset."));
    this->ui->ResetButton->setEnabled(false);
}
