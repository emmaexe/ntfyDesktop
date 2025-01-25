#include "HistorySettingsDialog.hpp"

#include "ui_HistorySettingsDialog.h"

HistorySettingsDialog::HistorySettingsDialog(QWidget* parent): QDialog(parent), ui(new Ui::HistorySettingsDialog) {
    this->ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(this->ui->applyButton, &QToolButton::clicked, this, &HistorySettingsDialog::applyButton);
}

HistorySettingsDialog::~HistorySettingsDialog() { delete ui; }

void HistorySettingsDialog::applyButton() { this->accept(); }
