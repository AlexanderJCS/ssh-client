#ifndef QTTEST2_SHELL_H
#define QTTEST2_SHELL_H

#include <string>
#include <libssh/libssh.h>
#include <QtCore>

class RemoteShell : public QObject {
    Q_OBJECT
private:
    bool running;
    ssh_session session;
    ssh_channel shellChannel;

    static ssh_session connect(const std::string& ip, const std::string& username, const std::string& password);
    static ssh_channel openShell(ssh_session session);
    void disconnect();
    void readOutput();

public:
    RemoteShell(const std::string& ip, const std::string& username, const std::string& password, QObject* parent);
//    RemoteShell(const std::string& ip, const std::string& username, const std::string& password);
    ~RemoteShell();

    std::string executeCommand(const std::string& command);

public slots:
    void start();
    void stop();

signals:
    void newOutput(const QString& output);
};


#endif // QTTEST2_SHELL_H
