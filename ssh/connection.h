#ifndef QTTEST2_CONNECTION_H
#define QTTEST2_CONNECTION_H

#include <string>
#include <libssh/libssh.h>

class SSHConnection {
private:
    ssh_session session;

    static ssh_session connect(const std::string& ip, const std::string& username, const std::string& password);
    void disconnect();

public:
    SSHConnection(const std::string& ip, const std::string& username, const std::string& password);
    ~SSHConnection();

    std::string executeCommand(const std::string& command);
};


#endif // QTTEST2_CONNECTION_H
