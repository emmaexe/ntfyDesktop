#pragma once

#include <QDialog>
#include <QButtonGroup>

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
    private:
        Ui::PreferencesDialog* ui;
        QButtonGroup* sourceButtons;
        static const std::array<std::string, 4> modes;
        static const std::array<std::string, 7> recentModes;
        static const std::array<std::string, 2> sourceModes;
        int mode = 0, numberValue = 5000, recentValue = 2, recentMode = 4, sourceMode = 0;
        bool startupNotifications = true, errorNotifications = true;
        void fetchFromConfig();
        void updateToConfig();
};
