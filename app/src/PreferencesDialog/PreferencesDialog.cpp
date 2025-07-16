#include "PreferencesDialog.hpp"

#include "../Config/Config.hpp"
#include "../Util/Util.hpp"
#include "ui_PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget* parent): QDialog(parent), ui(new Ui::PreferencesDialog) {
    this->ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->fetchFromConfig();

    QObject::connect(this->ui->saveButton, &QToolButton::clicked, this, &PreferencesDialog::saveButton);

    this->ui->aNumberLabel->hide();
    this->ui->aNumberSpinBox->setValue(this->numberValue);
    this->ui->aNumberSpinBox->hide();

    this->ui->recentLabel->hide();
    this->ui->recentSpinBox->setValue(this->recentValue);
    this->ui->recentSpinBox->hide();
    this->ui->recentComboBox->setCurrentIndex(this->recentMode);
    this->ui->recentComboBox->hide();

    this->ui->sourcePickerIndividual->hide();
    this->ui->sourcePickerGlobal->hide();
    this->sourceButtons = new QButtonGroup(this);
    this->sourceButtons->addButton(this->ui->sourcePickerIndividual, 0);
    this->sourceButtons->addButton(this->ui->sourcePickerGlobal, 1);
    this->sourceButtons->button(this->sourceMode)->setChecked(true);

    this->ui->startupNotificationCheckbox->setChecked(this->startupNotifications);
    this->ui->errorNotificationCheckbox->setChecked(this->errorNotifications);

    QObject::connect(this->ui->keepHistoryComboBox, &QComboBox::currentIndexChanged, this, &PreferencesDialog::keepHistoryChanged);
    this->ui->keepHistoryComboBox->setCurrentIndex(this->mode);
}

PreferencesDialog::~PreferencesDialog() { delete ui; }

void PreferencesDialog::saveButton() {
    this->mode = this->ui->keepHistoryComboBox->currentIndex();
    this->numberValue = this->ui->aNumberSpinBox->value();
    this->recentValue = this->ui->recentSpinBox->value();
    this->recentMode = this->ui->recentComboBox->currentIndex();
    this->sourceMode = this->sourceButtons->checkedId();
    this->startupNotifications = this->ui->startupNotificationCheckbox->checkState() == Qt::CheckState::Checked;
    this->errorNotifications = this->ui->errorNotificationCheckbox->checkState() == Qt::CheckState::Checked;
    this->updateToConfig();
    this->accept();
}

void PreferencesDialog::keepHistoryChanged(int index) {
    if (index == 1) {
        this->ui->aNumberLabel->show();
        this->ui->aNumberSpinBox->show();
        this->ui->recentLabel->hide();
        this->ui->recentSpinBox->hide();
        this->ui->recentComboBox->hide();
        this->ui->sourcePickerIndividual->show();
        this->ui->sourcePickerGlobal->show();
    } else if (index == 2) {
        this->ui->aNumberLabel->hide();
        this->ui->aNumberSpinBox->hide();
        this->ui->recentLabel->show();
        this->ui->recentSpinBox->show();
        this->ui->recentComboBox->show();
        this->ui->sourcePickerIndividual->hide();
        this->ui->sourcePickerGlobal->hide();
    } else {
        this->ui->aNumberLabel->hide();
        this->ui->aNumberSpinBox->hide();
        this->ui->recentLabel->hide();
        this->ui->recentSpinBox->hide();
        this->ui->recentComboBox->hide();
        this->ui->sourcePickerIndividual->hide();
        this->ui->sourcePickerGlobal->hide();
    }
}

const std::array<std::string, 4> PreferencesDialog::modes = { "All", "Number", "Recent", "None" };
const std::array<std::string, 7> PreferencesDialog::recentModes = { "Seconds", "Minutes", "Hours", "Days", "Weeks", "Months", "Years" };
const std::array<std::string, 2> PreferencesDialog::sourceModes = { "Individual", "All" };

void PreferencesDialog::fetchFromConfig() {
    Config::read();
    try {
        nlohmann::json preferences = Config::data()["preferences"];
        if (preferences.is_object()) {
            if (preferences["history"].is_object() && preferences["history"]["mode"].is_string()) {
                std::string mode = preferences["history"]["mode"].get<std::string>();
                Util::Strings::toLower(mode);
                if (mode == "all") {
                    this->mode = 0;
                } else if (mode == "number" && preferences["history"]["numberValue"].is_number()) {
                    this->mode = 1;
                    this->numberValue = preferences["history"]["numberValue"].get<int>();
                } else if (mode == "recent" && preferences["history"]["recentValue"].is_number() && preferences["history"]["recentMode"].is_string()) {
                    this->mode = 2;
                    this->recentValue = preferences["history"]["recentValue"].get<int>();
                    std::string recentMode = preferences["history"]["recentMode"].get<std::string>();
                    Util::Strings::toLower(recentMode);
                    if (recentMode == "seconds") {
                        this->recentMode = 0;
                    } else if (recentMode == "minutes") {
                        this->recentMode = 1;
                    } else if (recentMode == "hours") {
                        this->recentMode = 2;
                    } else if (recentMode == "days") {
                        this->recentMode = 3;
                    } else if (recentMode == "weeks") {
                        this->recentMode = 4;
                    } else if (recentMode == "months") {
                        this->recentMode = 5;
                    } else if (recentMode == "years") {
                        this->recentMode = 6;
                    }
                } else if (mode == "none") {
                    this->mode = 3;
                }
                if (preferences["history"]["sourceMode"].is_string()) {
                    std::string mode = preferences["history"]["sourceMode"].get<std::string>();
                    Util::Strings::toLower(mode);
                    if (mode == "individual") {
                        this->sourceMode = 0;
                    } else if (mode == "all") {
                        this->sourceMode = 1;
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
        }
    } catch (const nlohmann::json::type_error& ignored) {}
}

void PreferencesDialog::updateToConfig() {
    nlohmann::json preferences = nlohmann::json::object();
    preferences["history"]["mode"] = this->modes[this->mode];
    preferences["history"]["numberValue"] = this->numberValue;
    preferences["history"]["recentValue"] = this->recentValue;
    preferences["history"]["recentMode"] = this->recentModes[this->recentMode];
    preferences["history"]["sourceMode"] = this->sourceModes[this->sourceMode];
    preferences["notifications"]["startup"] = this->startupNotifications;
    preferences["notifications"]["error"] = this->errorNotifications;
    Config::data()["preferences"] = preferences;
    Config::write();
}
