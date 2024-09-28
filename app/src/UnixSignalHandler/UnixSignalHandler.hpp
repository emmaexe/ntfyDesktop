#pragma once

#include <signal.h>
#include <sys/socket.h>

#include <QObject>
#include <QSocketNotifier>
#include <functional>
#include <vector>

class UnixSignalHandler: public QObject {
        Q_OBJECT
    public:
        UnixSignalHandler(std::function<void(int)> fun, QObject* parent = nullptr);
        static void hupSignalHandler(int unused);
        static void intSignalHandler(int unused);
        static void termSignalHandler(int unused);
    public slots:
        void handleSigHup();
        void handleSigInt();
        void handleSigTerm();
    private:
        std::function<void(int)> fun;
        static int sighupFileDescriptor[2];
        static int sigintFileDescriptor[2];
        static int sigtermFileDescriptor[2];
        QSocketNotifier* sighupNotifier;
        QSocketNotifier* sigintNotifier;
        QSocketNotifier* sigtermNotifier;
};
