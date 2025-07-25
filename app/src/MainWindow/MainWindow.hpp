#pragma once

#include "../Config/Config.hpp"
#include "../ThreadManager/ThreadManager.hpp"
#include "../Util/ParsedURL.hpp"
#include "ConfigTab.hpp"

#include <KHelpMenu>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <memory>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow: public QMainWindow {
        Q_OBJECT
    public:
        MainWindow(std::shared_ptr<ThreadManager> threadManager, KAboutData& aboutData, QWidget* parent = nullptr);
        ~MainWindow();
    public slots:
        void ntfyProtocolTriggered(ParsedURL url);
        void saveAction();
        void addAction();
        void removeAction();
        void exitAction();
        void trayIconPressed(QSystemTrayIcon::ActivationReason reason);
        void showHideAction();
        void preferencesAction();
        void restartAction();
        void importAction();
        void historyAction();
        void pullButton();
        void pullButtonResults(const bool success);
    protected:
        void closeEvent(QCloseEvent* event) override;
        void changeEvent(QEvent* event) override;
    private:
        int newTabCounter = 1;
        std::vector<ConfigTab*> tabs;
        std::shared_ptr<ThreadManager> threadManager;
        Ui::MainWindow* ui;
        QSystemTrayIcon* tray;
        QMenu* trayMenu;
        QAction* showHideQAction;
        KHelpMenu* helpMenu;
        QTimer* pullButtonTimer;
};

class NotificationPuller: public QObject {
        Q_OBJECT
    public:
        NotificationPuller(const nlohmann::json& sources);
    public slots:
        void run();
    signals:
        void complete(const bool success);
    private:
        const nlohmann::json sources;
};
