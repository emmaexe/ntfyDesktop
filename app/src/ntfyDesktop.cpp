#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <KAboutData>
#include <KLocalizedString>
#include <curl/curl.h>
#include <iostream>
#include <optional>

#include "MainWindow/MainWindow.hpp"
#include "ErrorWindow/ErrorWindow.hpp"
#include "Config/Config.hpp"
#include "SingleInstanceManager/SingleInstanceManager.hpp"
#include "ThreadManager/ThreadManager.hpp"
#include "NotificationManager/NotificationManager.hpp"
#include "ntfyDesktop.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("moe.emmaexe.ntfyDesktop");
    KAboutData aboutData(QStringLiteral("ntfyDesktop"), i18n("Ntfy Desktop"), QStringLiteral(ND_VERSION), i18n("A client for the ntfy notification service."), KAboutLicense::GPL_V3, i18n("(c) 2024"), i18n("Some text..."), QStringLiteral("https://github.com/emmaexe/ntfyDesktop"), QStringLiteral("https://github.com/emmaexe/ntfyDesktop/issues"));
    aboutData.setDesktopFileName("moe.emmaexe.ntfyDesktop");
    aboutData.addAuthor(i18n("emmaexe"), i18n("Author"), QStringLiteral(""), QStringLiteral("https://www.emmaexe.moe/"), QStringLiteral(""));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;

    parser.addPositionalArgument("url", "Optional URL argument. Used by x-scheme-handler to handle the ntfy:// scheme.", "[url]");

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    std::optional<std::string> passedUrl = std::nullopt;

    if (!parser.positionalArguments().empty()) {
        passedUrl = parser.positionalArguments().first().toStdString();
    }

    SingleInstanceManager singleInstanceManager([&](std::optional<std::string> url){ std::cerr << "A new instance was started, but this instance does not have a main window to show." << std::endl; }, passedUrl);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::shared_ptr<QMainWindow> window;
    std::shared_ptr<ThreadManager> threadManager;
    if (Config::ready()) {
        threadManager = std::make_shared<ThreadManager>();
        window = std::make_shared<MainWindow>(threadManager);
        singleInstanceManager.onNewInstanceStarted = [&](std::optional<std::string> url){
            if (url.has_value()) {
                // TODO: Create new tab with the url to handle the ntfy:// protocol
            } else {
                if (window->isHidden()) {
                    window->show();
                } else {
                    QApplication::alert(window.get());
                }
            }
        };
    } else {
        window = std::make_shared<ErrorWindow>();
    }

    return app.exec();
}
