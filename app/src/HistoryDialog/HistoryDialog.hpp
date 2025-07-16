#pragma once

#include "NotificationListItem.hpp"

#include <QDialog>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
    class HistoryDialog;
}
QT_END_NAMESPACE

class HistoryDialog: public QDialog {
        Q_OBJECT
    public:
        HistoryDialog(QWidget* parent = nullptr);
        ~HistoryDialog();
    public slots:
        void deleteButton();
        void selectionChanged();
    private:
        Ui::HistoryDialog* ui;
        std::vector<std::unique_ptr<NotificationListItem>> notifications = {};
        std::vector<std::unique_ptr<QListWidgetItem>> listWidgetItems = {};
};
