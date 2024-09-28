#include "UnixSignalHandler.hpp"

int UnixSignalHandler::sighupFileDescriptor[2] = {};
int UnixSignalHandler::sigintFileDescriptor[2] = {};
int UnixSignalHandler::sigtermFileDescriptor[2] = {};

UnixSignalHandler::UnixSignalHandler(std::function<void(int)> fun, QObject* parent): fun(fun), QObject(parent) {
    // Register SigHup
    socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalHandler::sighupFileDescriptor);
    this->sighupNotifier = new QSocketNotifier(UnixSignalHandler::sighupFileDescriptor[1], QSocketNotifier::Read, this);
    connect(this->sighupNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigHup()));

    struct sigaction hup;
    hup.sa_handler = UnixSignalHandler::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;
    if (sigaction(SIGHUP, &hup, 0)) {
        //throw error
    }

    // Register SigInt
    socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalHandler::sigintFileDescriptor);
    this->sigintNotifier = new QSocketNotifier(UnixSignalHandler::sigintFileDescriptor[1], QSocketNotifier::Read, this);
    connect(this->sigintNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigInt()));

    struct sigaction intr;
    intr.sa_handler = UnixSignalHandler::intSignalHandler;
    sigemptyset(&intr.sa_mask);
    intr.sa_flags = 0;
    intr.sa_flags |= SA_RESTART;
    if (sigaction(SIGINT, &intr, 0)) {
        //throw error
    }

    // Register SigTerm
    socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalHandler::sigtermFileDescriptor);
    this->sigtermNotifier = new QSocketNotifier(UnixSignalHandler::sigtermFileDescriptor[1], QSocketNotifier::Read, this);
    connect(this->sigtermNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigTerm()));

    struct sigaction term;
    term.sa_handler = UnixSignalHandler::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESTART;
    if (sigaction(SIGTERM, &term, 0)) {
        //throw error
    }
}

void UnixSignalHandler::hupSignalHandler(int) {
    char a = 1;
    write(UnixSignalHandler::sighupFileDescriptor[0], &a, sizeof(a));
}

void UnixSignalHandler::intSignalHandler(int) {
    char a = 1;
    write(UnixSignalHandler::sigintFileDescriptor[0], &a, sizeof(a));
}

void UnixSignalHandler::termSignalHandler(int) {
    char a = 1;
    write(UnixSignalHandler::sigtermFileDescriptor[0], &a, sizeof(a));
}

void UnixSignalHandler::handleSigHup() {
    this->sighupNotifier->setEnabled(false);
    char tmp;
    read(this->sighupFileDescriptor[1], &tmp, sizeof(tmp));

    this->fun(SIGHUP);

    this->sighupNotifier->setEnabled(true);
}

void UnixSignalHandler::handleSigInt() {
    this->sigintNotifier->setEnabled(false);
    char tmp;
    read(this->sigintFileDescriptor[1], &tmp, sizeof(tmp));

    this->fun(SIGINT);

    this->sigintNotifier->setEnabled(true);
}

void UnixSignalHandler::handleSigTerm() {
    this->sigtermNotifier->setEnabled(false);
    char tmp;
    read(this->sigtermFileDescriptor[1], &tmp, sizeof(tmp));

    this->fun(SIGTERM);

    this->sigtermNotifier->setEnabled(true);
}

