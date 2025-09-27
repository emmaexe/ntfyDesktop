#include "ImportDialog.hpp"

#include "../Config/Config.hpp"
#include "../Util/ParsedURL.hpp"
#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"
#include "ui_ImportDialog.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <fstream>

ImportDialog::ImportDialog(QWidget* parent): QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint), ui(new Ui::ImportDialog) {
    this->ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(this->ui->fileSelectButton, &QToolButton::clicked, this, &ImportDialog::fileSelectButton);
    QObject::connect(this->ui->applyButton, &QToolButton::clicked, this, &ImportDialog::applyButton);
}

ImportDialog::~ImportDialog() { delete this->ui; }

void ImportDialog::fileSelectButton() {
    this->ui->statusLabel->setStyleSheet("font-weight: bold; color: " + Util::Colors::textColor().name() + ";");
    this->ui->statusLabel->setText("Loading backup...");
    QFileDialog* fileDialog = new QFileDialog(this, "Select Ntfy Android backup", QStandardPaths::writableLocation(QStandardPaths::HomeLocation), "JSON files (*.json)");
    fileDialog->setFileMode(QFileDialog::FileMode::ExistingFile);
    if (fileDialog->exec()) {
        std::ifstream fileStream(fileDialog->selectedFiles().at(0).toStdString());
        if (fileStream.is_open()) {
            try {
                nlohmann::json data = nlohmann::json::parse(fileStream);
                if (data["magic"] == "ntfy2586") {
                    this->internalTempConfig = nlohmann::json::object();
                    this->internalTempConfig["version"] = ND_VERSION;
                    this->internalTempConfig["sources"] = nlohmann::json::array();
                    int entryCounter = 1;

                    for (nlohmann::json source: data["subscriptions"]) {
                        ParsedURL parsedUrl(source["baseUrl"].get<std::string>());
                        nlohmann::json entry = nlohmann::json::object();
                        entry["name"] = source.contains("displayName") ? std::string(source["displayName"]) : "Imported Notification Source " + std::to_string(entryCounter);
                        entry["domain"] = parsedUrl.domain();
                        entry["topic"] = source["topic"];
                        entry["protocol"] = (parsedUrl.protocol() == "http" || parsedUrl.protocol() == "https" || parsedUrl.protocol() == "ws" || parsedUrl.protocol() == "wss") ? parsedUrl.protocol() : "https";
                        this->internalTempConfig["sources"].push_back(entry);
                        entryCounter++;
                    }

                    this->fileSuccess("Backup loaded successfully. Press the apply button to merge it into the current config.", this->internalTempConfig.dump());
                } else {
                    this->fileFailure("The file you selected is not a valid Ntfy Android backup file.");
                }
            } catch (const nlohmann::json::parse_error& e) { this->fileFailure("Failed to parse JSON file."); } catch (const nlohmann::json::type_error& e) {
                this->fileFailure("Failed to parse JSON file.");
            } catch (const nlohmann::json::other_error& e) { this->fileFailure("Failed to parse JSON file."); }
        } else {
            this->fileFailure("Failed to open file.");
        }
        fileStream.close();
    } else {
        this->fileFailure("No file selected.");
    }
    delete fileDialog;
}

void ImportDialog::applyButton() {
    for (nlohmann::json source: this->internalTempConfig["sources"]) {
        bool seen = false;
        for (nlohmann::json compareTarget: Config::data()["sources"]) {
            if (source["domain"] == compareTarget["domain"] && source["topic"] == compareTarget["topic"]) {
                seen = true;
                break;
            }
        }
        if (seen) { continue; }
        Config::data()["sources"].push_back(source);
    }
    Config::write();
    this->accept();
}

void ImportDialog::fileSuccess(const std::string& text, const std::string& data) {
    this->ui->statusLabel->setStyleSheet("font-weight: bold; color: " + Util::Colors::textColorSuccess().name() + ";");
    this->ui->statusLabel->setText(QString::fromStdString(text));
    this->ui->applyButton->setEnabled(true);
    this->ui->dataPlainTextEdit->setPlainText(QString::fromStdString(data));
}

void ImportDialog::fileFailure(const std::string& text) {
    this->ui->statusLabel->setStyleSheet("font-weight: bold; color: " + Util::Colors::textColorFailure().name() + ";");
    this->ui->statusLabel->setText(QString::fromStdString(text));
    this->ui->applyButton->setEnabled(false);
    this->ui->dataPlainTextEdit->setPlainText("");
}
