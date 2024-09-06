#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <vector>
#include <memory>

#include "ConfigTab.hpp"
#include "../Config/Config.hpp"
#include "../ThreadManager/ThreadManager.hpp"
#include "../ProtocolHandler/ProtocolHandler.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow: public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(std::shared_ptr<ThreadManager> threadManager, QWidget* parent = nullptr);
        ~MainWindow();
    public slots:
        void ntfyProtocolTriggered(ProtocolHandler url);
        void saveAction();
        void addAction();
        void removeAction();
        void exitAction();
        void aboutAction();
        void trayIconPressed(QSystemTrayIcon::ActivationReason reason);
        void showHideAction();
        void restartConfigAction();
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
        QAction* restartConfigQAction;
};
