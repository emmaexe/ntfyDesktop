#pragma once

#include "../Config/Config.hpp"

#include <QMainWindow>
#include <KHelpMenu>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ErrorWindow;
}
QT_END_NAMESPACE

class ErrorWindow: public QMainWindow {
        Q_OBJECT
    public:
        ErrorWindow(KAboutData& aboutData, QWidget* parent = nullptr);
        ~ErrorWindow();
    public slots:
        void resetConfig();
    private:
        Ui::ErrorWindow* ui;
        KHelpMenu* helpMenu;
};
