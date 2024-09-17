#include "ErrorWindow.hpp"

#include "ntfyDesktop.hpp"
#include "ui_ErrorWindow.h"

#include <QIcon>

ErrorWindow::ErrorWindow(KAboutData& aboutData, QWidget* parent): QMainWindow(parent), ui(new Ui::ErrorWindow) {
    this->ui->setupUi(this);

    this->ui->ErrorText->setText(QLabel::tr(Config::getError().c_str()));

    QObject::connect(this->ui->ResetButton, &QPushButton::clicked, this, &ErrorWindow::resetConfig);

    this->helpMenu = new KHelpMenu(this, aboutData);
    this->helpMenu->action(KHelpMenu::MenuId::menuAboutApp)->setIcon(QIcon(QStringLiteral(":/icons/ntfyDesktop.svg")));
    this->ui->menuBar->addMenu(this->helpMenu->menu());
    this->show();
}

ErrorWindow::~ErrorWindow() { delete ui; }

void ErrorWindow::resetConfig() {
    Config::reset();
    this->ui->ErrorText->setText(QLabel::tr("The configuration was reset."));
    this->ui->ResetButton->setEnabled(false);
}
