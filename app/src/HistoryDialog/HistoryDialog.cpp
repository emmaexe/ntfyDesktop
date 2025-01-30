#include "HistoryDialog.hpp"

#include "../Config/Config.hpp"
#include "../DataBase/DataBase.hpp"
#include "../Util/Util.hpp"
#include "./HistorySettingsDialog.hpp"
#include "ui_HistoryDialog.h"

#include <nlohmann/json.hpp>

HistoryDialog::HistoryDialog(QWidget* parent): QDialog(parent), ui(new Ui::HistoryDialog) {
    this->ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(this->ui->historySettingsButton, &QToolButton::clicked, this, &HistoryDialog::historySettingsButton);
    QObject::connect(this->ui->deleteButton, &QToolButton::clicked, this, &HistoryDialog::deleteButton);
    QObject::connect(this->ui->notificationList, &QListWidget::itemSelectionChanged, this, &HistoryDialog::selectionChanged);

    DataBase db;

    if (db.hasRows("Notifications")) {
        this->ui->notificationList->show();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, false);

        this->notifications = db.getNotificationsAsListItem(this);
        for (int i = 0; i < this->notifications.size(); i++) {
            std::unique_ptr<NotificationListItem>& item = this->notifications.at(i);
            std::unique_ptr<QListWidgetItem> listWidgetItem = std::make_unique<QListWidgetItem>(this->ui->notificationList);
            listWidgetItem->setSizeHint(item->sizeHint());
            listWidgetItem->setData(Qt::UserRole, i);
            QObject::connect(
                item.get(),
                &NotificationListItem::sizeChanged,
                this,
                [this, i](const QSize sizeHint) {
                    this->listWidgetItems.at(i)->setSizeHint(sizeHint);
                    this->ui->notificationList->update();
                },
                Qt::QueuedConnection
            );
            this->ui->notificationList->addItem(listWidgetItem.get());
            this->ui->notificationList->setItemWidget(listWidgetItem.get(), item.get());
            this->listWidgetItems.push_back(std::move(listWidgetItem));
        }
    } else {
        this->ui->notificationList->hide();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, true);
    }
}

HistoryDialog::~HistoryDialog() { delete ui; }

void HistoryDialog::historySettingsButton() {
    HistorySettingsDialog* dialog = new HistorySettingsDialog(this);
    dialog->exec();
}

void HistoryDialog::deleteButton() {
    QList<QListWidgetItem*> items = this->ui->notificationList->selectedItems();

    if (items.size() == 1) {
        int pos = items.at(0)->data(Qt::UserRole).toInt();
        std::unique_ptr<NotificationListItem>& widget = this->notifications.at(pos);
        DataBase db;
        db.deleteNotification(widget->id());
        this->ui->notificationList->takeItem(this->ui->notificationList->row(items.at(0)));
        this->listWidgetItems.erase(this->listWidgetItems.begin() + pos);
        this->notifications.erase(this->notifications.begin() + pos);
        for (int j = pos; j < this->listWidgetItems.size(); j++) {
            this->listWidgetItems.at(j)->setData(Qt::UserRole, j);
        }
    } else {
        std::vector<std::string> ids(items.size(), "");
        for (int i = 0; i < items.size(); i++) {
            int pos = items.at(i)->data(Qt::UserRole).toInt();
            ids[i] = this->notifications.at(pos)->id();
            this->ui->notificationList->takeItem(this->ui->notificationList->row(items.at(i)));
            this->listWidgetItems.erase(this->listWidgetItems.begin() + pos);
            this->notifications.erase(this->notifications.begin() + pos);
            for (int j = pos; j < this->listWidgetItems.size(); j++) {
                this->listWidgetItems.at(j)->setData(Qt::UserRole, j);
            }
        }
        DataBase db;
        db.deleteNotifications(ids);
    }

    if (this->ui->notificationList->count() == 0) {
        this->ui->notificationList->hide();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, true);
    }
}

void HistoryDialog::selectionChanged() { this->ui->deleteButton->setEnabled(this->ui->notificationList->selectedItems().size() != 0); }
