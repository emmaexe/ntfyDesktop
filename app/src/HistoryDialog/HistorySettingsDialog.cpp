#include "HistorySettingsDialog.hpp"

#include "../Config/Config.hpp"
#include "../Util/Util.hpp"
#include "ui_HistorySettingsDialog.h"

HistorySettingsDialog::HistorySettingsDialog(QWidget* parent): QDialog(parent), ui(new Ui::HistorySettingsDialog) {
    this->ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->fetchFromConfig();

    QObject::connect(this->ui->applyButton, &QToolButton::clicked, this, &HistorySettingsDialog::applyButton);

    this->ui->aNumberLabel->hide();
    this->ui->aNumberSpinBox->setValue(this->numberValue);
    this->ui->aNumberSpinBox->hide();

    this->ui->recentLabel->hide();
    this->ui->recentSpinBox->setValue(this->recentValue);
    this->ui->recentSpinBox->hide();
    this->ui->recentComboBox->setCurrentIndex(this->recentMode);
    this->ui->recentComboBox->hide();

    QObject::connect(this->ui->keepHistoryComboBox, &QComboBox::currentIndexChanged, this, &HistorySettingsDialog::keepHistoryChanged);
    this->ui->keepHistoryComboBox->setCurrentIndex(this->mode);
}

HistorySettingsDialog::~HistorySettingsDialog() { delete ui; }

void HistorySettingsDialog::applyButton() {
    this->mode = this->ui->keepHistoryComboBox->currentIndex();
    this->numberValue = this->ui->aNumberSpinBox->value();
    this->recentValue = this->ui->recentSpinBox->value();
    this->recentMode = this->ui->recentComboBox->currentIndex();
    this->updateToConfig();
    this->accept();
}

void HistorySettingsDialog::keepHistoryChanged(int index) {
    if (index == 1) {
        this->ui->aNumberLabel->show();
        this->ui->aNumberSpinBox->show();
        this->ui->recentLabel->hide();
        this->ui->recentSpinBox->hide();
        this->ui->recentComboBox->hide();
    } else if (index == 2) {
        this->ui->aNumberLabel->hide();
        this->ui->aNumberSpinBox->hide();
        this->ui->recentLabel->show();
        this->ui->recentSpinBox->show();
        this->ui->recentComboBox->show();
    } else {
        this->ui->aNumberLabel->hide();
        this->ui->aNumberSpinBox->hide();
        this->ui->recentLabel->hide();
        this->ui->recentSpinBox->hide();
        this->ui->recentComboBox->hide();
    }
}

const std::array<std::string, 4> HistorySettingsDialog::modes = {"All", "Number", "Recent", "None"};
const std::array<std::string, 7> HistorySettingsDialog::recentModes = {"Seconds", "Minutes", "Hours", "Days", "Weeks", "Months", "Years"};

void HistorySettingsDialog::fetchFromConfig() {
    Config::read();
    try {
        nlohmann::json historyConfig = Config::data()["history"];
        if (historyConfig.is_object() && historyConfig["mode"].is_string()) {
            std::string mode = historyConfig["mode"].get<std::string>();
            Util::toLower(mode);
            if (mode == "all") {
                this->mode = 0;
            } else if (mode == "number" && historyConfig["numberValue"].is_number()) {
                this->mode = 1;
                this->numberValue = historyConfig["numberValue"].get<int>();
            } else if (mode == "recent" && historyConfig["recentValue"].is_number() && historyConfig["recentMode"].is_string()) {
                this->mode = 2;
                this->recentValue = historyConfig["recentValue"].get<int>();
                std::string recentMode = historyConfig["recentMode"].get<std::string>();
                Util::toLower(recentMode);
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
        }
    } catch (const nlohmann::json::type_error& ignored) {}
}

void HistorySettingsDialog::updateToConfig() {
    nlohmann::json historyConfig = nlohmann::json::object();
    historyConfig["mode"] = this->modes[this->mode];
    historyConfig["numberValue"] = this->numberValue;
    historyConfig["recentValue"] = this->recentValue;
    historyConfig["recentMode"] = this->recentModes[this->recentMode];
    Config::data()["history"] = historyConfig;
    Config::write();
}
