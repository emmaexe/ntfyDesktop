#pragma once

#include <signal.h>
#include <sys/socket.h>

#include <QObject>
#include <QSocketNotifier>
#include <functional>
#include <vector>

/**
 * @brief A singleton that bridges unix and Qt signals.
 */
class UnixSignalBridge: public QObject {
        Q_OBJECT
        Q_DISABLE_COPY(UnixSignalBridge)
    public:
        /**
         * @brief Get the UnixSignalBridge instance
         */
        static UnixSignalBridge* get();
    signals:
        /**
         * @brief Connect to this signal to recieve unix signals
         *
         * @param signal The signal code that was recieved as per `signal.h`
         */
        void signal(int);
    private slots:
        void handleSigHup();
        void handleSigInt();
        void handleSigTerm();
    private:
        UnixSignalBridge(QObject* parent = nullptr);
        static int sighupFileDescriptor[2];
        static int sigintFileDescriptor[2];
        static int sigtermFileDescriptor[2];
        QSocketNotifier* sighupNotifier;
        QSocketNotifier* sigintNotifier;
        QSocketNotifier* sigtermNotifier;
};
