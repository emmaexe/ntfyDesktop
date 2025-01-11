#include "ntfyDesktop.hpp"

#include "Config/Config.hpp"
#include "ErrorWindow/ErrorWindow.hpp"
#include "MainWindow/MainWindow.hpp"
#include "NotificationManager/NotificationManager.hpp"
#include "SingleInstanceManager/SingleInstanceManager.hpp"
#include "ThreadManager/ThreadManager.hpp"
#include "UnixSignalHandler/UnixSignalHandler.hpp"
#include "Util/ParsedURL.hpp"

#include <curl/curl.h>
#include <signal.h>

#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("moe.emmaexe.ntfyDesktop");
    KAboutData aboutData(
        QStringLiteral("moe.emmaexe.ntfyDesktop"),
        QStringLiteral("Ntfy Desktop"),
        QStringLiteral(ND_VERSION),
        i18n(ND_DESCRIPTION_SUMMARY),
        KAboutLicense::GPL_V3,
        QStringLiteral("© 2024"),
        QStringLiteral(),
        QStringLiteral(ND_HOMEPAGE_URL),
        QStringLiteral(ND_ISSUES_URL)
    );
    aboutData.setProgramLogo(QIcon(":/icons/ntfyDesktop.svg"));
    aboutData.setDesktopFileName("moe.emmaexe.ntfyDesktop");
    aboutData.addAuthor(QStringLiteral("emmaexe"), i18n("Author"), QStringLiteral("contact@emmaexe.moe"), QStringLiteral("https://www.emmaexe.moe/"), QStringLiteral(""));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addPositionalArgument("url", "Optional URL argument. Used by x-scheme-handler to handle the ntfy:// protocol.", "[url]");

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    std::optional<std::string> passedUrl = std::nullopt;
    if (!parser.positionalArguments().empty()) { passedUrl = parser.positionalArguments().first().toStdString(); }
    SingleInstanceManager singleInstanceManager(
        [&](std::optional<std::string> url) { std::cerr << "A new instance was started, but this instance does not have a main window to show." << std::endl; }, passedUrl
    );

    std::srand(std::time(0));
    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::shared_ptr<QMainWindow> window;
    std::shared_ptr<ThreadManager> threadManager;
    std::shared_ptr<UnixSignalHandler> signalHandler;
    if (Config::ready()) {
        threadManager = std::make_shared<ThreadManager>();
        window = std::make_shared<MainWindow>(threadManager, aboutData);
        singleInstanceManager.onNewInstanceStarted = [&](std::optional<std::string> url) {
            if (url.has_value()) {
                try {
                    std::static_pointer_cast<MainWindow>(window).get()->ntfyProtocolTriggered(ParsedURL(url.value()));
                } catch (ParsedURLException e) {
                    NotificationManager::errorNotification("An invalid url was passed to ntfyDesktop", e.what());
                }
            } else {
                if (window->isHidden()) {
                    window->show();
                } else {
                    QApplication::alert(window.get());
                }
            }
        };
        signalHandler = std::make_shared<UnixSignalHandler>(
            [threadManager, window](int signal) {
                if (signal == SIGTERM || signal == SIGINT || signal == SIGHUP) {
                    std::static_pointer_cast<MainWindow>(window).get()->hide();
                    threadManager->stopAll();
                    QApplication::quit();
                }
            },
            window.get()
        );
    } else {
        window = std::make_shared<ErrorWindow>(aboutData);
    }

    return app.exec();
}
