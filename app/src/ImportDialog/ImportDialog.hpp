#pragma once

#include <nlohmann/json.hpp>

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ImportDialog;
}
QT_END_NAMESPACE

class ImportDialog: public QDialog {
        Q_OBJECT
    public:
        ImportDialog(QWidget* parent = nullptr);
        ~ImportDialog();
    public slots:
        void fileSelectButton();
        void applyButton();
    private:
        void fileSuccess(const std::string& text, const std::string& data);
        void fileFailure(const std::string& text);
        nlohmann::json internalTempConfig;
        Ui::ImportDialog* ui;
};
