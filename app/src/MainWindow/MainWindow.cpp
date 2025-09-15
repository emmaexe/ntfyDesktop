#include "MainWindow.hpp"

#include "../DataBase/DataBase.hpp"
#include "../HistoryDialog/HistoryDialog.hpp"
#include "../ImportDialog/ImportDialog.hpp"
#include "../NotificationManager/NotificationManager.hpp"
#include "../PreferencesDialog/PreferencesDialog.hpp"
#include "../Util/Util.hpp"
#include "ui_MainWindow.h"

#include <QCloseEvent>
#include <QCryptographicHash>
#include <QEvent>
#include <QIcon>
#include <QPushButton>
#include <algorithm>
#include <iostream>

using Util::Colors::ColorMode;

MainWindow::MainWindow(std::shared_ptr<ThreadManager> threadManager, KAboutData& aboutData, QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow), threadManager(threadManager) {
    this->ui->setupUi(this);
    QObject::connect(this->ui->saveAction, &QAction::triggered, this, &MainWindow::saveAction);
    QObject::connect(this->ui->addAction, &QAction::triggered, this, &MainWindow::addAction);
    QObject::connect(this->ui->removeAction, &QAction::triggered, this, &MainWindow::removeAction);
    QObject::connect(this->ui->historyAction, &QAction::triggered, this, &MainWindow::historyAction);
    QObject::connect(this->ui->importAction, &QAction::triggered, this, &MainWindow::importAction);
    QObject::connect(this->ui->restartAction, &QAction::triggered, this, &MainWindow::restartAction);
    QObject::connect(this->ui->preferencesAction, &QAction::triggered, this, &MainWindow::preferencesAction);
    QObject::connect(this->ui->exitAction, &QAction::triggered, this, &MainWindow::exitAction);
    QObject::connect(this->ui->pullButton, &QPushButton::clicked, this, &MainWindow::pullButton);

    {
        DataBase db;
        nlohmann::json sources = Config::data()["sources"];
        for (int i = 0; i < sources.size(); i++) {
            this->tabs.push_back(
                new ConfigTab(sources[i]["name"], sources[i]["protocol"], sources[i]["domain"], sources[i]["topic"], db.getAuth(Util::topicHash(sources[i]["domain"], sources[i]["topic"])), this)
            );
            this->ui->tabs->addTab(this->tabs.at(i), this->tabs.at(i)->getName().c_str());
        }
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

    this->pullButtonTimer = new QTimer(this);
    this->pullButtonTimer->setSingleShot(true);
    QObject::connect(this->pullButtonTimer, &QTimer::timeout, [this](){
        Util::Colors::setButtonColor(*this->ui->pullButton, ColorMode::Normal);
    });

    if (this->tabs.size() == 0) {
        this->show();
        this->ui->statusBar->showMessage(QStatusBar::tr("Ready"), 2000);
    } else {
        NotificationManager::startupNotification();
    }
}

MainWindow::~MainWindow() { delete ui, tray; }

void MainWindow::ntfyProtocolTriggered(ParsedURL url) {
    if (url.protocol() == "ntfy") {
        this->tabs.push_back(new ConfigTab("New Notification Source " + std::to_string(this->newTabCounter), url.protocol(), url.domain(), url.path()[0], AuthConfig(), this));
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
    nlohmann::json previous = Config::data()["sources"];
    Config::data()["sources"] = nlohmann::json::array();
    std::map<std::string, AuthConfig> seen;

    for (int i = 0; i < this->ui->tabs->count(); i++) {
        ConfigTab* tab = static_cast<ConfigTab*>(this->ui->tabs->widget(i));
        std::string topicHash = Util::topicHash(tab->getDomain(), tab->getTopic());
        if (seen.find(topicHash) != seen.end()) {
            std::string tabName = "⚠️" + tab->getName();
            this->ui->tabs->setTabText(i, QString::fromStdString(tabName));
            this->ui->tabs->setCurrentIndex(i);
            this->ui->statusBar->showMessage(QStatusBar::tr("⚠️ Duplicate configuration found. Did not save."), 5000);
            Config::data()["sources"] = previous;
            return;
        } else if (!Util::isDomain(tab->getDomain()) || !Util::isTopic(tab->getTopic())) {
            std::string tabName = "⚠️" + tab->getName();
            this->ui->tabs->setTabText(i, QString::fromStdString(tabName));
            this->ui->tabs->setCurrentIndex(i);
            this->ui->statusBar->showMessage(QStatusBar::tr("⚠️ Invalid domain or topic. Did not save."), 5000);
            Config::data()["sources"] = previous;
            return;
        } else {
            seen[topicHash] = tab->getAuth();
            this->ui->tabs->setTabText(i, tab->getName().c_str());
            nlohmann::json tabData;
            tabData["name"] = tab->getName();
            tabData["domain"] = tab->getDomain();
            tabData["topic"] = tab->getTopic();
            tabData["protocol"] = tab->getProtocol();
            Config::data()["sources"].push_back(tabData);
        }
    }
    for (int i = 0; i < this->ui->tabs->count(); i++) {
        ConfigTab* tab = static_cast<ConfigTab*>(this->ui->tabs->widget(i));
        tab->clearInvisible();
    }

    Config::write();
    DataBase db;
    db.multiSetAuth(seen);

    this->ui->statusBar->showMessage(QStatusBar::tr("Configuration saved."), 2000);
}

void MainWindow::addAction() {
    if (this->tabs.size() == 0) {
        this->ui->tabs->show();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, false);
    }
    this->tabs.push_back(new ConfigTab("New Notification Source " + std::to_string(this->newTabCounter), "https", "", "", AuthConfig(), this));
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

void MainWindow::preferencesAction() {
    PreferencesDialog* dialog = new PreferencesDialog(this);
    dialog->exec();
}

void MainWindow::restartAction() {
    bool wasShown = !this->isHidden();
    if (wasShown) {
        this->hide();
        QApplication::processEvents();
    }

    this->tabs.clear();
    this->ui->tabs->clear();

    DataBase db;
    Config::read();

    nlohmann::json sources = Config::data()["sources"];
    for (int i = 0; i < sources.size(); i++) {
        this->tabs.push_back(
            new ConfigTab(sources[i]["name"], sources[i]["protocol"], sources[i]["domain"], sources[i]["topic"], db.getAuth(Util::topicHash(sources[i]["domain"], sources[i]["topic"])), this)
        );
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

void MainWindow::importAction() {
    ImportDialog* dialog = new ImportDialog(this);
    if (dialog->exec()) {
        this->restartAction();
        this->ui->statusBar->showMessage("Ntfy Android backup merged into existing config.", 5000);
    }
}

void MainWindow::historyAction() {
    HistoryDialog* dialog = new HistoryDialog(this);
    dialog->exec();
}

void MainWindow::pullButton() {
    this->pullButtonTimer->stop();
    Util::Colors::setButtonColor(*this->ui->pullButton, ColorMode::Normal);
    this->ui->pullButton->setEnabled(false);

    QThread* thread = new QThread;
    NotificationPuller* notificationPuller = new NotificationPuller(Config::data()["sources"]);
    notificationPuller->moveToThread(thread);

    connect(thread, &QThread::started, notificationPuller, &NotificationPuller::run);
    connect(notificationPuller, &NotificationPuller::complete, this, &MainWindow::pullButtonResults);

    connect(notificationPuller, &NotificationPuller::complete, notificationPuller, &NotificationPuller::deleteLater);
    connect(notificationPuller, &NotificationPuller::complete, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void MainWindow::pullButtonResults(const bool success) {
    Util::Colors::setButtonColor(*this->ui->pullButton, (success ? ColorMode::Success : ColorMode::Failure));
    this->ui->pullButton->setEnabled(true);
    this->pullButtonTimer->start(5000);
}

NotificationPuller::NotificationPuller(const nlohmann::json& sources): sources(sources) {}

void NotificationPuller::run() {
    bool error = false;
    std::vector<std::unique_ptr<NtfyThread>> threads;
    std::mutex mutex;
    if (this->sources.is_array()) {
        DataBase db;
        for (const nlohmann::json& source: this->sources) {
            try {
                int lastTimestamp = -1;
                std::string name = source["name"].get<std::string>(), protocol = source["protocol"].get<std::string>(), domain = source["domain"].get<std::string>(), topic = source["topic"].get<std::string>(), topicHash = Util::topicHash(domain, topic);
                AuthConfig authConfig = db.getAuth(topicHash);

                bool verifyTls = db.getTlsVerificationPreference();
                std::string CAPath = db.getCAPathPreference();

                threads.push_back(std::make_unique<NtfyThread>(name, protocol, domain, topic, authConfig, -1, verifyTls, CAPath, std::nullopt, std::nullopt, &mutex, true));
            } catch (const nlohmann::json::out_of_range& e) { std::cerr << "Invalid source in config, ignoring: " << source << std::endl; }
        }
    }
    for (std::unique_ptr<NtfyThread>& thread: threads) {
        thread->waitForStop();
        error = error || thread->hasError();
    }
    emit complete(!error);
}
