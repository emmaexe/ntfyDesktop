#include "MainWindow.hpp"

#include "../NotificationManager/NotificationManager.hpp"
#include "../Util/Util.hpp"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QEvent>
#include <QIcon>
#include <algorithm>

MainWindow::MainWindow(std::shared_ptr<ThreadManager> threadManager, KAboutData& aboutData, QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow), threadManager(threadManager) {
    this->ui->setupUi(this);
    QObject::connect(this->ui->saveAction, &QAction::triggered, this, &MainWindow::saveAction);
    QObject::connect(this->ui->addAction, &QAction::triggered, this, &MainWindow::addAction);
    QObject::connect(this->ui->removeAction, &QAction::triggered, this, &MainWindow::removeAction);
    QObject::connect(this->ui->restartAction, &QAction::triggered, this, &MainWindow::restartAction);
    QObject::connect(this->ui->exitAction, &QAction::triggered, this, &MainWindow::exitAction);

    nlohmann::json sources = Config::data()["sources"];
    for (int i = 0; i < sources.size(); i++) {
        this->tabs.push_back(new ConfigTab(sources[i]["name"], sources[i]["server"], sources[i]["topic"], this));
        this->ui->tabs->addTab(this->tabs.at(i), this->tabs.at(i)->getName().c_str());
    }

    if (this->tabs.size() == 0) {
        this->ui->tabs->hide();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, true);
    } else {
        this->ui->tabs->show();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, false);
    }

    this->trayMenu = new QMenu(this);

    this->showHideQAction = new QAction(QIcon(":/icons/ntfy-symbolic.svg"), QAction::tr("Show/hide window"), this);
    QObject::connect(this->showHideQAction, &QAction::triggered, this, &MainWindow::showHideAction);
    this->trayMenu->addAction(this->showHideQAction);
    this->trayMenu->addAction(this->ui->restartAction);
    this->trayMenu->addAction(this->ui->exitAction);

    this->tray = new QSystemTrayIcon(this);
    this->tray->setIcon(QIcon(":/icons/ntfy-symbolic.svg").pixmap(236, 236));
    this->tray->setContextMenu(this->trayMenu);
    this->tray->show();
    QObject::connect(this->tray, &QSystemTrayIcon::activated, this, &MainWindow::trayIconPressed);

    this->helpMenu = new KHelpMenu(this, aboutData);
    this->helpMenu->action(KHelpMenu::MenuId::menuAboutApp)->setIcon(QIcon(QStringLiteral(":/icons/ntfyDesktop.svg")));
    this->ui->menuBar->addMenu(this->helpMenu->menu());

    if (this->tabs.size() == 0) {
        this->show();
        this->ui->statusBar->showMessage(QStatusBar::tr("Ready"), 2000);
    } else {
        NotificationManager::startupNotification();
    }
}

MainWindow::~MainWindow() { delete ui, tray; }

void MainWindow::ntfyProtocolTriggered(ProtocolHandler url) {
    if (url.protocol() == "ntfy") {
        this->tabs.push_back(new ConfigTab("New Notification Source " + std::to_string(this->newTabCounter), url.domain(), url.path()[0], this));
        this->newTabCounter++;
        this->ui->tabs->addTab(this->tabs.at(this->tabs.size() - 1), this->tabs.at(this->tabs.size() - 1)->getName().c_str());
        this->ui->tabs->setCurrentIndex(this->tabs.size() - 1);
        if (this->isHidden()) {
            this->show();
        } else {
            QApplication::alert(this);
        }
    }
}

void MainWindow::saveAction() {
    Config::data()["sources"] = nlohmann::json::array();
    std::vector<std::string> seen = {};
    for (int i = 0; i < this->tabs.size(); i++) {
        ConfigTab* tab = this->tabs.at(i);
        std::string domainTopic = tab->getServer() + "/" + tab->getTopic();
        if (std::find(seen.begin(), seen.end(), domainTopic) != seen.end()) {
            std::string tabName = "⚠️" + tab->getName();
            this->ui->tabs->setTabText(i, QString::fromStdString(tabName));
            this->ui->tabs->setCurrentIndex(i);
            this->ui->statusBar->showMessage(QStatusBar::tr("⚠️ Duplicate configuration found. Did not save."), 5000);
            return;
        } else if (!Util::isDomain(tab->getServer()) || !Util::isTopic(tab->getTopic())) {
            std::string tabName = "⚠️" + tab->getName();
            this->ui->tabs->setTabText(i, QString::fromStdString(tabName));
            this->ui->tabs->setCurrentIndex(i);
            this->ui->statusBar->showMessage(QStatusBar::tr("⚠️ Invalid domain or topic. Did not save."), 5000);
            return;
        } else {
            seen.push_back(domainTopic);
            this->ui->tabs->setTabText(i, tab->getName().c_str());
            nlohmann::json tabData;
            tabData["name"] = tab->getName();
            tabData["server"] = tab->getServer();
            tabData["topic"] = tab->getTopic();
            Config::data()["sources"].push_back(tabData);
        }
    }
    Config::write();
    this->ui->statusBar->showMessage(QStatusBar::tr("Configuration saved."), 2000);
}

void MainWindow::addAction() {
    if (this->tabs.size() == 0) {
        this->ui->tabs->show();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, false);
    }
    this->tabs.push_back(new ConfigTab("New Notification Source " + std::to_string(this->newTabCounter), "", "", this));
    this->newTabCounter++;
    this->ui->tabs->addTab(this->tabs.at(this->tabs.size() - 1), this->tabs.at(this->tabs.size() - 1)->getName().c_str());
    this->ui->tabs->setCurrentIndex(this->tabs.size() - 1);
}

void MainWindow::removeAction() {
    if (this->ui->tabs->count() > 0) {
        int i = this->ui->tabs->currentIndex();
        this->ui->tabs->removeTab(i);
        delete this->tabs.at(i);
        this->tabs.erase(this->tabs.begin() + i);
    }
    if (this->tabs.size() == 0) {
        this->ui->tabs->hide();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, true);
    }
}

void MainWindow::exitAction() {
    if (!this->isHidden()) {
        this->hide();
        QApplication::processEvents();
    }
    this->threadManager->stopAll();
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    event->ignore();
    this->hide();
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMinimized()) { this->hide(); }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::trayIconPressed(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::MiddleClick:
            if (this->isHidden()) {
                this->show();
            } else {
                this->hide();
            }
            break;
        default: break;
    }
}

void MainWindow::showHideAction() {
    if (this->isHidden()) {
        this->show();
    } else {
        this->hide();
    }
}

void MainWindow::restartAction() {
    bool wasShown = !this->isHidden();
    if (wasShown) {
        this->hide();
        QApplication::processEvents();
    }

    this->tabs.clear();
    this->ui->tabs->clear();

    Config::read();

    nlohmann::json sources = Config::data()["sources"];
    for (int i = 0; i < sources.size(); i++) {
        this->tabs.push_back(new ConfigTab(sources[i]["name"], sources[i]["server"], sources[i]["topic"], this));
        this->ui->tabs->addTab(this->tabs.at(i), this->tabs.at(i)->getName().c_str());
    }

    if (this->tabs.size() == 0) {
        this->ui->tabs->hide();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, true);
    } else {
        this->ui->tabs->show();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, false);
    }

    this->threadManager->restartConfig();
    this->newTabCounter = 1;

    if (wasShown) { this->show(); }
}
