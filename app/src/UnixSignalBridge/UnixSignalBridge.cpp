#include "UnixSignalBridge.hpp"

#include "../Util/Logging.hpp"

#include <QApplication>

int UnixSignalBridge::sighupFileDescriptor[2] = {};
int UnixSignalBridge::sigintFileDescriptor[2] = {};
int UnixSignalBridge::sigtermFileDescriptor[2] = {};

UnixSignalBridge* UnixSignalBridge::get() {
    static UnixSignalBridge* instance = new UnixSignalBridge(QApplication::instance());
    return instance;
}

void UnixSignalBridge::handleSigHup() {
    this->sighupNotifier->setEnabled(false);
    char tmp;
    ssize_t n = read(this->sighupFileDescriptor[1], &tmp, sizeof(tmp));

    emit signal(SIGHUP);

    this->sighupNotifier->setEnabled(true);
}

void UnixSignalBridge::handleSigInt() {
    this->sigintNotifier->setEnabled(false);
    char tmp;
    ssize_t n = read(this->sigintFileDescriptor[1], &tmp, sizeof(tmp));

    emit signal(SIGINT);

    this->sigintNotifier->setEnabled(true);
}

void UnixSignalBridge::handleSigTerm() {
    this->sigtermNotifier->setEnabled(false);
    char tmp;
    ssize_t n = read(this->sigtermFileDescriptor[1], &tmp, sizeof(tmp));

    emit signal(SIGTERM);

    this->sigtermNotifier->setEnabled(true);
}

UnixSignalBridge::UnixSignalBridge(QObject* parent): QObject(parent) {
    Logger& logger = Logger::get();

    // Register SigHup
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalBridge::sighupFileDescriptor)) {
        logger.error("Unable to handle SIGHUP: Failed to create socketpair");
    } else {
        this->sighupNotifier = new QSocketNotifier(UnixSignalBridge::sighupFileDescriptor[1], QSocketNotifier::Read, this);
        connect(this->sighupNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigHup()));

        struct sigaction hup;
        hup.sa_handler = +[](int) {
            char a = 1;
            ssize_t n = write(UnixSignalBridge::sighupFileDescriptor[0], &a, sizeof(a));
        };
        sigemptyset(&hup.sa_mask);
        hup.sa_flags = 0;
        hup.sa_flags |= SA_RESTART;
        if (sigaction(SIGHUP, &hup, 0)) { logger.error("Unable to handle SIGHUP: Failed to call sigaction"); }
    }

    // Register SigInt
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalBridge::sigintFileDescriptor)) {
        logger.error("Unable to handle SIGINT: Failed to create socketpair");
    } else {
        this->sigintNotifier = new QSocketNotifier(UnixSignalBridge::sigintFileDescriptor[1], QSocketNotifier::Read, this);
        connect(this->sigintNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigInt()));

        struct sigaction intr;
        intr.sa_handler = +[](int) {
            char a = 1;
            ssize_t n = write(UnixSignalBridge::sigintFileDescriptor[0], &a, sizeof(a));
        };
        sigemptyset(&intr.sa_mask);
        intr.sa_flags = 0;
        intr.sa_flags |= SA_RESTART;
        if (sigaction(SIGINT, &intr, 0)) { logger.error("Unable to handle SIGINT: Failed to call sigaction"); }
    }

    // Register SigTerm
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalBridge::sigtermFileDescriptor)) {
        logger.error("Unable to handle SIGTERM: Failed to create socketpair");
    } else {
        this->sigtermNotifier = new QSocketNotifier(UnixSignalBridge::sigtermFileDescriptor[1], QSocketNotifier::Read, this);
        connect(this->sigtermNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigTerm()));

        struct sigaction term;
        term.sa_handler = +[](int) {
            char a = 1;
            ssize_t n = write(UnixSignalBridge::sigtermFileDescriptor[0], &a, sizeof(a));
        };
        sigemptyset(&term.sa_mask);
        term.sa_flags = 0;
        term.sa_flags |= SA_RESTART;
        if (sigaction(SIGTERM, &term, 0)) { logger.error("Unable to handle SIGTERM: Failed to call sigaction"); }
    }
}
