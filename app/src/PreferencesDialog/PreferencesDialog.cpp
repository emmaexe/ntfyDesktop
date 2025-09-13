#include "PreferencesDialog.hpp"

#include "../Config/Config.hpp"
#include "../DataBase/DataBase.hpp"
#include "../Util/Util.hpp"
#include "ui_PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget* parent): QDialog(parent), ui(new Ui::PreferencesDialog) {
    this->ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->fetchStored();

    QObject::connect(this->ui->saveButton, &QToolButton::clicked, this, &PreferencesDialog::saveButton);

    this->ui->aNumberLabel->hide();
    this->ui->aNumberSpinBox->setValue(this->historyNumberValue);
    this->ui->aNumberSpinBox->hide();

    this->ui->recentLabel->hide();
    this->ui->recentSpinBox->setValue(this->historyRecentValue);
    this->ui->recentSpinBox->hide();
    this->ui->recentComboBox->setCurrentIndex(this->historyRecentMode);
    this->ui->recentComboBox->hide();

    this->ui->sourcePickerIndividual->hide();
    this->ui->sourcePickerGlobal->hide();
    this->sourceButtons = new QButtonGroup(this);
    this->sourceButtons->addButton(this->ui->sourcePickerIndividual, 0);
    this->sourceButtons->addButton(this->ui->sourcePickerGlobal, 1);
    this->sourceButtons->button(this->historySourceMode)->setChecked(true);

    QObject::connect(this->ui->keepHistoryComboBox, &QComboBox::currentIndexChanged, this, &PreferencesDialog::keepHistoryChanged);
    this->ui->keepHistoryComboBox->setCurrentIndex(this->historyMode);
    this->keepHistoryChanged(this->historyMode);

    this->ui->retryNumberLabel->hide();
    this->ui->retryNumberSpinBox->setValue(this->reconnectNumberValue);
    this->ui->retryNumberSpinBox->hide();
    this->ui->timeoutLabel->hide();
    this->ui->timeoutComboBox->setCurrentIndex(this->reconnectTimeoutMode);
    this->ui->timeoutComboBox->hide();
    this->ui->timeoutSpinBox->setValue(this->reconnectTimeoutValue);
    this->ui->timeoutSpinBox->hide();

    QObject::connect(this->ui->retryComboBox, &QComboBox::currentIndexChanged, this, &PreferencesDialog::retryChanged);
    this->ui->retryComboBox->setCurrentIndex(this->reconnectMode);
    this->retryChanged(this->reconnectMode);

    this->ui->startupNotificationCheckbox->setChecked(this->startupNotifications);
    this->ui->errorNotificationCheckbox->setChecked(this->errorNotifications);

    this->ui->tlsVerificationCheckBox->setChecked(this->tlsVerification);
    this->ui->CALineEdit->setText(QString::fromStdString(this->customCaPath));
}

PreferencesDialog::~PreferencesDialog() { delete ui; }

void PreferencesDialog::saveButton() {
    this->historyMode = this->ui->keepHistoryComboBox->currentIndex();
    this->historyNumberValue = this->ui->aNumberSpinBox->value();
    this->historyRecentValue = this->ui->recentSpinBox->value();
    this->historyRecentMode = this->ui->recentComboBox->currentIndex();
    this->historySourceMode = this->sourceButtons->checkedId();
    this->reconnectMode = this->ui->retryComboBox->currentIndex();
    this->reconnectNumberValue = this->ui->retryNumberSpinBox->value();
    this->reconnectTimeoutMode = this->ui->timeoutComboBox->currentIndex();
    this->reconnectTimeoutValue = this->ui->timeoutSpinBox->value();
    this->startupNotifications = this->ui->startupNotificationCheckbox->checkState() == Qt::CheckState::Checked;
    this->errorNotifications = this->ui->errorNotificationCheckbox->checkState() == Qt::CheckState::Checked;
    this->tlsVerification = this->ui->tlsVerificationCheckBox->checkState() == Qt::CheckState::Checked;
    this->customCaPath = this->ui->CALineEdit->text().toStdString();
    this->storeChanges();
    this->accept();
}

void PreferencesDialog::keepHistoryChanged(int index) {
    switch (index) {
    case 1: {
        this->ui->aNumberLabel->show();
        this->ui->aNumberSpinBox->show();
        this->ui->recentLabel->hide();
        this->ui->recentSpinBox->hide();
        this->ui->recentComboBox->hide();
        this->ui->sourcePickerIndividual->show();
        this->ui->sourcePickerGlobal->show();
        break;
    }
    case 2: {
        this->ui->aNumberLabel->hide();
        this->ui->aNumberSpinBox->hide();
        this->ui->recentLabel->show();
        this->ui->recentSpinBox->show();
        this->ui->recentComboBox->show();
        this->ui->sourcePickerIndividual->hide();
        this->ui->sourcePickerGlobal->hide();
        break;
    }
    default: {
        this->ui->aNumberLabel->hide();
        this->ui->aNumberSpinBox->hide();
        this->ui->recentLabel->hide();
        this->ui->recentSpinBox->hide();
        this->ui->recentComboBox->hide();
        this->ui->sourcePickerIndividual->hide();
        this->ui->sourcePickerGlobal->hide();
        break;
    }
    }
}

void PreferencesDialog::retryChanged(int index) {
    switch (index) {
    case 0: {
        this->ui->retryNumberLabel->hide();
        this->ui->retryNumberSpinBox->hide();
        this->ui->timeoutLabel->show();
        this->ui->timeoutComboBox->show();
        this->ui->timeoutSpinBox->show();
        break;
    }
    case 1: {
        this->ui->retryNumberLabel->show();
        this->ui->retryNumberSpinBox->show();
        this->ui->timeoutLabel->show();
        this->ui->timeoutComboBox->show();
        this->ui->timeoutSpinBox->show();
        break;
    }
    case 2: {
        this->ui->retryNumberLabel->hide();
        this->ui->retryNumberSpinBox->hide();
        this->ui->timeoutLabel->hide();
        this->ui->timeoutComboBox->hide();
        this->ui->timeoutSpinBox->hide();
        break;
    }
    default: {
        break;
    }
    }
}

const std::array<std::string, 4> PreferencesDialog::historyModes = { "All", "Number", "Recent", "None" };
const std::array<std::string, 7> PreferencesDialog::historyRecentModes = { "Seconds", "Minutes", "Hours", "Days", "Weeks", "Months", "Years" };
const std::array<std::string, 2> PreferencesDialog::historySourceModes = { "Individual", "All" };
const std::array<std::string, 3> PreferencesDialog::reconnectModes = { "Forever", "Number", "Never" };
const std::array<std::string, 7> PreferencesDialog::reconnectTimeoutModes = { "Seconds", "Minutes", "Hours", "Days", "Weeks", "Months", "Years" };


void PreferencesDialog::fetchStored() {
    Config::read();
    try {
        nlohmann::json preferences = Config::data()["preferences"];
        if (preferences.is_object()) {
            if (preferences["history"].is_object() && preferences["history"]["mode"].is_string()) {
                std::string mode = preferences["history"]["mode"].get<std::string>();
                Util::Strings::toLower(mode);
                if (mode == "all") {
                    this->historyMode = 0;
                } else if (mode == "number" && preferences["history"]["numberValue"].is_number()) {
                    this->historyMode = 1;
                    this->historyNumberValue = preferences["history"]["numberValue"].get<int>();
                } else if (mode == "recent" && preferences["history"]["recentValue"].is_number() && preferences["history"]["recentMode"].is_string()) {
                    this->historyMode = 2;
                    this->historyRecentValue = preferences["history"]["recentValue"].get<int>();
                    std::string recentMode = preferences["history"]["recentMode"].get<std::string>();
                    Util::Strings::toLower(recentMode);
                    if (recentMode == "seconds") {
                        this->historyRecentMode = 0;
                    } else if (recentMode == "minutes") {
                        this->historyRecentMode = 1;
                    } else if (recentMode == "hours") {
                        this->historyRecentMode = 2;
                    } else if (recentMode == "days") {
                        this->historyRecentMode = 3;
                    } else if (recentMode == "weeks") {
                        this->historyRecentMode = 4;
                    } else if (recentMode == "months") {
                        this->historyRecentMode = 5;
                    } else if (recentMode == "years") {
                        this->historyRecentMode = 6;
                    }
                } else if (mode == "none") {
                    this->historyMode = 3;
                }
                if (preferences["history"]["sourceMode"].is_string()) {
                    std::string mode = preferences["history"]["sourceMode"].get<std::string>();
                    Util::Strings::toLower(mode);
                    if (mode == "individual") {
                        this->historySourceMode = 0;
                    } else if (mode == "all") {
                        this->historySourceMode = 1;
                    }
                }
            }
            if (preferences["notifications"].is_object()) {
                if (preferences["notifications"]["startup"].is_boolean()) {
                    this->startupNotifications = preferences["notifications"]["startup"];
                }
                if (preferences["notifications"]["error"].is_boolean()) {
                    this->errorNotifications= preferences["notifications"]["error"];
                }
            }
            if (preferences["reconnect"].is_object() && preferences["reconnect"]["mode"].is_string()) {
                std::string mode = preferences["reconnect"]["mode"].get<std::string>();
                Util::Strings::toLower(mode);
                if (mode == "forever" && preferences["reconnect"]["timeoutMode"].is_string() && preferences["reconnect"]["timeoutValue"].is_number()) {
                    this->reconnectMode = 0;
                    this->reconnectTimeoutValue = preferences["reconnect"]["timeoutValue"].get<int>();
                    std::string timeoutMode = preferences["reconnect"]["timeoutMode"].get<std::string>();
                    Util::Strings::toLower(timeoutMode);
                    if (timeoutMode == "seconds") {
                        this->reconnectTimeoutMode = 0;
                    } else if (timeoutMode == "minutes") {
                        this->reconnectTimeoutMode = 1;
                    } else if (timeoutMode == "hours") {
                        this->reconnectTimeoutMode = 2;
                    } else if (timeoutMode == "days") {
                        this->reconnectTimeoutMode = 3;
                    } else if (timeoutMode == "weeks") {
                        this->reconnectTimeoutMode = 4;
                    } else if (timeoutMode == "months") {
                        this->reconnectTimeoutMode = 5;
                    } else if (timeoutMode == "years") {
                        this->reconnectTimeoutMode = 6;
                    }
                } else if (mode == "number" && preferences["reconnect"]["timeoutMode"].is_string() && preferences["reconnect"]["timeoutValue"].is_number() && preferences["reconnect"]["numberValue"].is_number()) {
                    this->reconnectMode = 1;
                    this->reconnectTimeoutValue = preferences["reconnect"]["timeoutValue"].get<int>();
                    this->reconnectNumberValue = preferences["reconnect"]["numberValue"].get<int>();
                    std::string timeoutMode = preferences["reconnect"]["timeoutMode"].get<std::string>();
                    Util::Strings::toLower(timeoutMode);
                    if (timeoutMode == "seconds") {
                        this->reconnectTimeoutMode = 0;
                    } else if (timeoutMode == "minutes") {
                        this->reconnectTimeoutMode = 1;
                    } else if (timeoutMode == "hours") {
                        this->reconnectTimeoutMode = 2;
                    } else if (timeoutMode == "days") {
                        this->reconnectTimeoutMode = 3;
                    } else if (timeoutMode == "weeks") {
                        this->reconnectTimeoutMode = 4;
                    } else if (timeoutMode == "months") {
                        this->reconnectTimeoutMode = 5;
                    } else if (timeoutMode == "years") {
                        this->reconnectTimeoutMode = 6;
                    }
                } else if (mode == "never") {
                    this->reconnectMode = 2;
                }
            }
        }
    } catch (const nlohmann::json::type_error& ignored) {}
    DataBase db;
    this->tlsVerification = db.getTlsVerificationPreference();
    this->customCaPath = db.getCAPathPreference();
}

void PreferencesDialog::storeChanges() {
    nlohmann::json preferences = nlohmann::json::object();
    preferences["history"]["mode"] = this->historyModes[this->historyMode];
    preferences["history"]["numberValue"] = this->historyNumberValue;
    preferences["history"]["recentValue"] = this->historyRecentValue;
    preferences["history"]["recentMode"] = this->historyRecentModes[this->historyRecentMode];
    preferences["history"]["sourceMode"] = this->historySourceModes[this->historySourceMode];
    preferences["notifications"]["startup"] = this->startupNotifications;
    preferences["notifications"]["error"] = this->errorNotifications;
    preferences["reconnect"]["mode"] = this->reconnectModes[this->reconnectMode];
    preferences["reconnect"]["numberValue"] = this->reconnectNumberValue;
    preferences["reconnect"]["timeoutMode"] = this->reconnectTimeoutModes[this->reconnectTimeoutMode];
    preferences["reconnect"]["timeoutValue"] = this->reconnectTimeoutValue;
    Config::data()["preferences"] = preferences;
    Config::write();
    DataBase db;
    db.setTlsVerificationPreference(this->tlsVerification);
    db.setCAPathPreference(this->customCaPath);
}
