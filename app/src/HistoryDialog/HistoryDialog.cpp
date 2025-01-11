#include "HistoryDialog.hpp"

#include "../Config/Config.hpp"
#include "../DataBase/DataBase.hpp"
#include "../Util/Util.hpp"
#include "ui_HistoryDialog.h"

#include <nlohmann/json.hpp>

HistoryDialog::HistoryDialog(QWidget* parent): QDialog(parent), ui(new Ui::HistoryDialog) {
    this->ui->setupUi(this);
    QObject::connect(this->ui->historySettingsButton, &QToolButton::clicked, this, &HistoryDialog::historySettingsButton);
    QObject::connect(this->ui->clearHistoryButton, &QToolButton::clicked, this, &HistoryDialog::clearHistoryButton);

    DataBase db;

    if (db.hasRows("Notifications")) {
        this->ui->notificationList->show();
        Util::setLayoutVisibility(this->ui->noSourcesContainer, false);

        this->notifications = db.getAllNotifications(this);
        for (int i = 0; i < this->notifications.size(); i++) {
            std::unique_ptr<NotificationListItem>& item = this->notifications.at(i);
            std::unique_ptr<QListWidgetItem> listWidgetItem = std::make_unique<QListWidgetItem>(this->ui->notificationList);
            listWidgetItem->setSizeHint(item->sizeHint());
            QObject::connect(item.get(), &NotificationListItem::sizeChanged, this, [this, i](const QSize sizeHint){
                this->listWidgetItems.at(i)->setSizeHint(sizeHint);
                this->ui->notificationList->update();
            }, Qt::QueuedConnection);
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

void HistoryDialog::historySettingsButton() {}

void HistoryDialog::clearHistoryButton() {}
