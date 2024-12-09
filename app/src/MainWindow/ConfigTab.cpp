#include "ConfigTab.hpp"

#include "../Util/Util.hpp"
#include "ntfyDesktop.hpp"
#include "ui_ConfigTab.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <iostream>

ConfigTab::ConfigTab(std::string name, std::string domain, std::string topic, bool secure, QWidget* parent): QWidget(parent), ui(new Ui::ConfigTab) {
    this->ui->setupUi(this);
    this->ui->nameLineEdit->setText(QString::fromStdString(name));
    this->ui->domainLineEdit->setText(QString::fromStdString(domain));
    this->ui->topicLineEdit->setText(QString::fromStdString(topic));
    this->ui->secureCheckBox->setChecked(secure);

    this->ui->authUsernameLabel->hide();
    this->ui->authUsernameLineEdit->hide();
    this->ui->authPasswordLabel->hide();
    this->ui->authPasswordLineEdit->hide();
    this->ui->authTokenLabel->hide();
    this->ui->authTokenLineEdit->hide();

    this->ui->authTypeComboBox->addItems({ "None", "Username/Password", "Token" });
    this->ui->authTypeComboBox->setCurrentIndex(0);

    QObject::connect(this->ui->authTypeComboBox, &QComboBox::currentIndexChanged, this, &ConfigTab::authTypeChanged);

    QObject::connect(this->ui->testButton, &QToolButton::clicked, this, &ConfigTab::testButton);

    this->ui->testLabel->hide();
    this->testLabelTimer = new QTimer(this);
    this->testLabelTimer->setSingleShot(true);
    QObject::connect(this->testLabelTimer, &QTimer::timeout, this->ui->testLabel, &QLabel::hide);
}

ConfigTab::~ConfigTab() { delete ui; }

std::string ConfigTab::getName() { return this->ui->nameLineEdit->text().toStdString(); }

std::string ConfigTab::getDomain() { return this->ui->domainLineEdit->text().toStdString(); }

std::string ConfigTab::getTopic() { return this->ui->topicLineEdit->text().toStdString(); }

bool ConfigTab::getSecure() { return this->ui->secureCheckBox->isChecked(); }

AuthConfig ConfigTab::getAuth() {
    AuthConfig config;
    config.type = static_cast<AuthType>(this->ui->authTypeComboBox->currentIndex());
    config.username = this->ui->authUsernameLineEdit->text().toStdString();
    config.password = this->ui->authPasswordLineEdit->text().toStdString();
    config.token = this->ui->authTokenLineEdit->text().toStdString();
    return config;
}

void ConfigTab::testButton() {
    this->testLabelTimer->stop();
    this->ui->testButton->setEnabled(false);
    this->ui->testLabel->setStyleSheet("font-weight: bold; color: " + Util::Colors::textColor().name() + ";");
    this->ui->testLabel->setText("Testing connection...");
    this->ui->testLabel->show();

    QThread* thread = new QThread;
    ConnectionTester* tester = new ConnectionTester(this->getName(), this->getDomain(), this->getTopic(), this->getSecure());
    tester->moveToThread(thread);

    connect(thread, &QThread::started, tester, &ConnectionTester::runTest);
    connect(tester, &ConnectionTester::testFinished, this, &ConfigTab::testResults);

    connect(tester, &ConnectionTester::testFinished, tester, &ConnectionTester::deleteLater);
    connect(tester, &ConnectionTester::testFinished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void ConfigTab::testResults(const bool& result) {
    if (result) {
        this->ui->testLabel->setStyleSheet("font-weight: bold; color: " + Util::Colors::textColorSuccess().name() + ";");
        this->ui->testLabel->setText("Connection successful. Notification source is available.");
    } else {
        this->ui->testLabel->setStyleSheet("font-weight: bold; color: " + Util::Colors::textColorFailure().name() + ";");
        this->ui->testLabel->setText("Connection failed.");
    }
    this->ui->testLabel->show();
    this->ui->testButton->setEnabled(true);
    this->testLabelTimer->start(5000);
}

void ConfigTab::authTypeChanged(int index) {
    AuthType authType = static_cast<AuthType>(index);
    if (authType == AuthType::NONE) {
        this->ui->authUsernameLabel->hide();
        this->ui->authUsernameLineEdit->hide();
        this->ui->authPasswordLabel->hide();
        this->ui->authPasswordLineEdit->hide();
        this->ui->authTokenLabel->hide();
        this->ui->authTokenLineEdit->hide();
    } else if (authType == AuthType::USERNAME_PASSWORD) {
        this->ui->authUsernameLabel->show();
        this->ui->authUsernameLineEdit->show();
        this->ui->authPasswordLabel->show();
        this->ui->authPasswordLineEdit->show();
        this->ui->authTokenLabel->hide();
        this->ui->authTokenLineEdit->hide();
    } else if (authType == AuthType::TOKEN) {
        this->ui->authUsernameLabel->hide();
        this->ui->authUsernameLineEdit->hide();
        this->ui->authPasswordLabel->hide();
        this->ui->authPasswordLineEdit->hide();
        this->ui->authTokenLabel->show();
        this->ui->authTokenLineEdit->show();
    }
}

ConnectionTester::ConnectionTester(const std::string& name, const std::string& domain, const std::string& topic, const bool& secure): name(name), domain(domain), topic(topic), secure(secure) {}

void ConnectionTester::runTest() {
    this->testSuccessful = false;
    std::string url = (this->secure ? "https://" : "http://") + this->domain + "/" + this->topic + "/json";

    CURL* curlHandle = curl_easy_init();
    char curlError[CURL_ERROR_SIZE] = "";

    curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, curlError);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &ConnectionTester::writeCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, ND_USERAGENT);
    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());

    curl_easy_perform(curlHandle);

    emit testFinished(this->testSuccessful);
}

size_t ConnectionTester::writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    ConnectionTester* this_p = static_cast<ConnectionTester*>(userdata);
    std::vector<std::string> data = Util::split(std::string(ptr, size * nmemb), "\n");

    for (std::string& line: data) {
        if (line.empty()) { continue; }
        try {
            nlohmann::json jsonData = nlohmann::json::parse(line);
            if (jsonData["event"] == "open") {
                this_p->testSuccessful = true;
                return 0;
            }
        } catch (nlohmann::json::parse_error ignored) {}
    }

    return size * nmemb;
}
