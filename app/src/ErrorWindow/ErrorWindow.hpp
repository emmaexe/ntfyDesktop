#pragma once

#include <QMainWindow>
#include "../Config/Config.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class ErrorWindow;
}
QT_END_NAMESPACE

class ErrorWindow: public QMainWindow {
    Q_OBJECT
    public:
        ErrorWindow(QWidget* parent = nullptr);
        ~ErrorWindow();
    public slots:
        void resetConfig();
    private:
        Ui::ErrorWindow* ui;
};

