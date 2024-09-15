#include "ntfyDesktop.hpp"

#include "Config/Config.hpp"
#include "ErrorWindow/ErrorWindow.hpp"
#include "MainWindow/MainWindow.hpp"
#include "ProtocolHandler/ProtocolHandler.hpp"
#include "SingleInstanceManager/SingleInstanceManager.hpp"
#include "ThreadManager/ThreadManager.hpp"

#include <curl/curl.h>

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
        QStringLiteral("(c) 2024"),
        QStringLiteral(),
        QStringLiteral(ND_HOMEPAGE_URL),
        QStringLiteral(ND_ISSUES_URL)
    );
    aboutData.addAuthor(
        QStringLiteral("emmaexe"),
        i18n("Author"),
        QStringLiteral(""),
        QStringLiteral("https://www.emmaexe.moe/"),
        QStringLiteral("https://cdn.discordapp.com/avatars/299265668522442752/841bddd42846b31f2a06eeb5e93a0652.png?size=4096")
    );
    KAboutData::setApplicationData(aboutData);

    QGuiApplication::setDesktopFileName("moe.emmaexe.ntfyDesktop.desktop");

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
    if (Config::ready()) {
        threadManager = std::make_shared<ThreadManager>();
        window = std::make_shared<MainWindow>(threadManager);
        singleInstanceManager.onNewInstanceStarted = [&](std::optional<std::string> url) {
            if (url.has_value()) {
                std::static_pointer_cast<MainWindow>(window).get()->ntfyProtocolTriggered(ProtocolHandler(url.value()));
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
