#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class HistorySettingsDialog;
}
QT_END_NAMESPACE

class HistorySettingsDialog: public QDialog {
        Q_OBJECT
    public:
        HistorySettingsDialog(QWidget* parent = nullptr);
        ~HistorySettingsDialog();
    public slots:
        void applyButton();
    private:
        Ui::HistorySettingsDialog* ui;
};
