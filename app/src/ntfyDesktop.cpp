#include "ntfyDesktop.hpp"

#include "Config/Config.hpp"
#include "ErrorWindow/ErrorWindow.hpp"
#include "MainWindow/MainWindow.hpp"
#include "NotificationManager/NotificationManager.hpp"
#include "SingleInstanceManager/SingleInstanceManager.hpp"
#include "ThreadManager/ThreadManager.hpp"
#include "UnixSignalBridge/UnixSignalBridge.hpp"
#include "Util/FileManager.hpp"
#include "Util/Logging.hpp"
#include "Util/ParsedURL.hpp"

#include <KAboutData>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
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
    aboutData.addAuthor(QStringLiteral("Emma"), i18n("Author"), QStringLiteral("contact@emmaexe.moe"), QStringLiteral("https://emmaexe.moe/"), QStringLiteral(""));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addPositionalArgument("url", "Optional URL argument. Used by x-scheme-handler to handle the ntfy:// protocol.", "[url]");

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    SingleInstanceManager::get()->init(
        !parser.positionalArguments().empty() ?
        std::make_optional(parser.positionalArguments().first()) :
        std::nullopt
    );

    if (Config::ready()) {
        ThreadManager* threadManager = new ThreadManager(&app);
        MainWindow* window = new MainWindow(threadManager, aboutData);
        QObject::connect(SingleInstanceManager::get(), &SingleInstanceManager::new_instance, [&](std::optional<QString> url){
            if (url.has_value()) {
                try {
                    window->ntfyProtocolTriggered(ParsedURL(url.value().toStdString()));
                } catch (ParsedURLException e) { NotificationManager::errorNotification("An invalid url was passed to ntfyDesktop", e.what()); }
            } else {
                if (window->isHidden()) {
                    window->show();
                } else {
                    app.alert(window);
                }
            }
        });
        QObject::connect(UnixSignalBridge::get(), &UnixSignalBridge::signal, [threadManager, window](int signal) {
            if (signal == SIGTERM || signal == SIGINT || signal == SIGHUP) {
                window->hide();
                threadManager->stopAll();
                QApplication::quit();
            }
        });
    } else {
        ErrorWindow* window = new ErrorWindow(aboutData);
    }

    return app.exec();
}
