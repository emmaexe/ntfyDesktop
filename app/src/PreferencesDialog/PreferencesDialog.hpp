#pragma once

#include <QButtonGroup>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class PreferencesDialog;
}
QT_END_NAMESPACE

class PreferencesDialog: public QDialog {
        Q_OBJECT
    public:
        PreferencesDialog(QWidget* parent = nullptr);
        ~PreferencesDialog();
    public slots:
        void saveButton();
        void keepHistoryChanged(int index);
        void retryChanged(int index);
    private:
        Ui::PreferencesDialog* ui;
        QButtonGroup* sourceButtons;
        static const std::array<std::string, 4> historyModes;
        static const std::array<std::string, 7> historyRecentModes;
        static const std::array<std::string, 2> historySourceModes;
        static const std::array<std::string, 3> reconnectModes;
        static const std::array<std::string, 7> reconnectTimeoutModes;
        int historyMode = 0, historyNumberValue = 5000, historyRecentValue = 2, historyRecentMode = 4, historySourceMode = 0, reconnectMode = 1, reconnectNumberValue = 3, reconnectTimeoutValue = 2, reconnectTimeoutMode = 1;
        bool startupNotifications = true, errorNotifications = true;
        void fetchFromConfig();
        void updateToConfig();
};
