#ifndef QTTEST2_SHELL_H
#define QTTEST2_SHELL_H

#include <string>
#include <libssh/libssh.h>

class RemoteShell {
private:
    ssh_session session;
    ssh_channel shell;

    static ssh_session connect(const std::string& ip, const std::string& username, const std::string& password);
    static ssh_channel openShell(ssh_session session);
    void disconnect();

public:
    RemoteShell(const std::string& ip, const std::string& username, const std::string& password);
    ~RemoteShell();

    std::string executeCommand(const std::string& command);
};


#endif // QTTEST2_SHELL_H
