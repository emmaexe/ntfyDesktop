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
        void keepHistoryChanged(int index);
    private:
        Ui::HistorySettingsDialog* ui;
        static const std::array<std::string, 4> modes;
        static const std::array<std::string, 7> recentModes;
        int mode = 0, numberValue = 5000, recentValue = 2, recentMode = 4;
        void fetchFromConfig();
        void updateToConfig();
};
