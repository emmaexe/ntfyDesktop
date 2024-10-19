#include "UnixSignalHandler.hpp"

#include <iostream>

int UnixSignalHandler::sighupFileDescriptor[2] = {};
int UnixSignalHandler::sigintFileDescriptor[2] = {};
int UnixSignalHandler::sigtermFileDescriptor[2] = {};

UnixSignalHandler::UnixSignalHandler(std::function<void(int)> fun, QObject* parent): fun(fun), QObject(parent) {
    // Register SigHup
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalHandler::sighupFileDescriptor)) {
        std::cerr << "Unable to handle SIGHUP: " << "Failed to create socketpair" << std::endl;
    } else {
        this->sighupNotifier = new QSocketNotifier(UnixSignalHandler::sighupFileDescriptor[1], QSocketNotifier::Read, this);
        connect(this->sighupNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigHup()));

        struct sigaction hup;
        hup.sa_handler = UnixSignalHandler::hupSignalHandler;
        sigemptyset(&hup.sa_mask);
        hup.sa_flags = 0;
        hup.sa_flags |= SA_RESTART;
        if (sigaction(SIGHUP, &hup, 0)) { std::cerr << "Unable to handle SIGHUP: " << "Failed to call sigaction" << std::endl; }
    }

    // Register SigInt
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalHandler::sigintFileDescriptor)) {
        std::cerr << "Unable to handle SIGINT: " << "Failed to create socketpair" << std::endl;
    } else {
        this->sigintNotifier = new QSocketNotifier(UnixSignalHandler::sigintFileDescriptor[1], QSocketNotifier::Read, this);
        connect(this->sigintNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigInt()));

        struct sigaction intr;
        intr.sa_handler = UnixSignalHandler::intSignalHandler;
        sigemptyset(&intr.sa_mask);
        intr.sa_flags = 0;
        intr.sa_flags |= SA_RESTART;
        if (sigaction(SIGINT, &intr, 0)) { std::cerr << "Unable to handle SIGINT: " << "Failed to call sigaction" << std::endl; }
    }

    // Register SigTerm
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, UnixSignalHandler::sigtermFileDescriptor)) {
        std::cerr << "Unable to handle SIGTERM: " << "Failed to create socketpair" << std::endl;
    } else {
        this->sigtermNotifier = new QSocketNotifier(UnixSignalHandler::sigtermFileDescriptor[1], QSocketNotifier::Read, this);
        connect(this->sigtermNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigTerm()));

        struct sigaction term;
        term.sa_handler = UnixSignalHandler::termSignalHandler;
        sigemptyset(&term.sa_mask);
        term.sa_flags = 0;
        term.sa_flags |= SA_RESTART;
        if (sigaction(SIGTERM, &term, 0)) { std::cerr << "Unable to handle SIGTERM: " << "Failed to call sigaction" << std::endl; }
    }
}

void UnixSignalHandler::hupSignalHandler(int) {
    char a = 1;
    ssize_t n = write(UnixSignalHandler::sighupFileDescriptor[0], &a, sizeof(a));
}

void UnixSignalHandler::intSignalHandler(int) {
    char a = 1;
    ssize_t n = write(UnixSignalHandler::sigintFileDescriptor[0], &a, sizeof(a));
}

void UnixSignalHandler::termSignalHandler(int) {
    char a = 1;
    ssize_t n = write(UnixSignalHandler::sigtermFileDescriptor[0], &a, sizeof(a));
}

void UnixSignalHandler::handleSigHup() {
    this->sighupNotifier->setEnabled(false);
    char tmp;
    ssize_t n = read(this->sighupFileDescriptor[1], &tmp, sizeof(tmp));

    this->fun(SIGHUP);

    this->sighupNotifier->setEnabled(true);
}

void UnixSignalHandler::handleSigInt() {
    this->sigintNotifier->setEnabled(false);
    char tmp;
    ssize_t n = read(this->sigintFileDescriptor[1], &tmp, sizeof(tmp));

    this->fun(SIGINT);

    this->sigintNotifier->setEnabled(true);
}

void UnixSignalHandler::handleSigTerm() {
    this->sigtermNotifier->setEnabled(false);
    char tmp;
    ssize_t n = read(this->sigtermFileDescriptor[1], &tmp, sizeof(tmp));

    this->fun(SIGTERM);

    this->sigtermNotifier->setEnabled(true);
}
