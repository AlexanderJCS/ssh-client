#include "shell.h"

#include <libssh/libssh.h>
#include <iostream>


namespace {
    int verifyKnownHost(ssh_session session) {
        // Source: https://api.libssh.org/master/libssh_tutor_guided_tour.html

        enum ssh_known_hosts_e state;
        unsigned char *hash = nullptr;
        ssh_key srv_pubkey = nullptr;
        size_t hlen;
        char buf[10];
        char *hexa;
        char *p;
        int cmp;
        int rc;

        rc = ssh_get_server_publickey(session, &srv_pubkey);
        if (rc < 0) {
            return -1;
        }

        rc = ssh_get_publickey_hash(srv_pubkey,
                                    SSH_PUBLICKEY_HASH_SHA1,
                                    &hash,
                                    &hlen);
        ssh_key_free(srv_pubkey);
        if (rc < 0) {
            return -1;
        }

        state = ssh_session_is_known_server(session);
        switch (state) {
            case SSH_KNOWN_HOSTS_OK:
                /* OK */

                break;
            case SSH_KNOWN_HOSTS_CHANGED:
                fprintf(stderr, "Host key for server changed: it is now:\n");
                ssh_print_hexa("Public key hash", hash, hlen);
                fprintf(stderr, "For security reasons, connection will be stopped\n");
                ssh_clean_pubkey_hash(&hash);

                return -1;
            case SSH_KNOWN_HOSTS_OTHER:
                fprintf(stderr, "The host key for this server was not found but an other"
                                "type of key exists.\n");
                fprintf(stderr, "An attacker might change the default server key to"
                                "confuse your client into thinking the key does not exist\n");
                ssh_clean_pubkey_hash(&hash);

                return -1;
            case SSH_KNOWN_HOSTS_NOT_FOUND:
                fprintf(stderr, "Could not find known host file.\n");
                fprintf(stderr, "If you accept the host key here, the file will be"
                                "automatically created.\n");

                /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */

            case SSH_KNOWN_HOSTS_UNKNOWN:
                hexa = ssh_get_hexa(hash, hlen);
                fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
                fprintf(stderr, "Public key hash: %s\n", hexa);
                ssh_string_free_char(hexa);
                ssh_clean_pubkey_hash(&hash);
                p = fgets(buf, sizeof(buf), stdin);
                if (p == nullptr) {
                    return -1;
                }

                cmp = strncasecmp(buf, "yes", 3);
                if (cmp != 0) {
                    return -1;
                }

                rc = ssh_session_update_known_hosts(session);
                if (rc < 0) {
                    fprintf(stderr, "Error %s\n", strerror(errno));
                    return -1;
                }

                break;
            case SSH_KNOWN_HOSTS_ERROR:
                fprintf(stderr, "Error %s", ssh_get_error(session));
                ssh_clean_pubkey_hash(&hash);
                return -1;
        }

        ssh_clean_pubkey_hash(&hash);
        return 0;
    }
}


RemoteShell::RemoteShell(const std::string& ip, const std::string& username, const std::string& password) {
    session = connect(ip, username, password);
    shellChannel = openShell(session);
}

RemoteShell::~RemoteShell() {
    disconnect();
}

ssh_session RemoteShell::connect(const std::string& ip, const std::string& username, const std::string& password) {
    ssh_session sshSession = ssh_new();
    if (sshSession == nullptr) {
        return nullptr;
    }

    ssh_options_set(sshSession, SSH_OPTIONS_HOST, ip.c_str());

    int rc = ssh_connect(sshSession);
    if (rc != SSH_OK) {
        ssh_free(sshSession);
        std::cerr << "Error connecting to " << ip << ": " << ssh_get_error(sshSession) << std::endl;
        return nullptr;
    }

    if (verifyKnownHost(sshSession) < 0) {
        ssh_disconnect(sshSession);
        ssh_free(sshSession);
        std::cerr << "Error verifying known host" << std::endl;
        return nullptr;
    }

    rc = ssh_userauth_password(sshSession, username.c_str(), password.c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        ssh_disconnect(sshSession);
        ssh_free(sshSession);
        std::cerr << "Error authenticating with " << ip << ": " << ssh_get_error(sshSession) << std::endl;
        return nullptr;
    }

    return sshSession;
}

#include <regex>

std::string RemoteShell::readOutput() {
    if (shellChannel == nullptr) {
        return "";
    }

    char buffer[1024];
    std::string output;

    int nbytes = ssh_channel_read(shellChannel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        output.append(buffer, nbytes);
        nbytes = ssh_channel_read(shellChannel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0) {
        std::cerr << "Error reading output" << std::endl;
        return "";
    }

    // Remove ANSI escape codes
    std::regex ansiEscapeCodeRegex(R"(\x1B\[[0-9;]*[A-Za-z])");
    output = std::regex_replace(output, ansiEscapeCodeRegex, "");

    return output;
}

ssh_channel RemoteShell::openShell(ssh_session session) {
    if (session == nullptr) {
        return nullptr;
    }

    ssh_channel channel = ssh_channel_new(session);
    if (channel == nullptr) {
        std::cerr << "Error creating channel: " << ssh_get_error(session) << std::endl;
        return nullptr;
    }

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        std::cerr << "Error opening channel: " << ssh_get_error(session) << std::endl;
        ssh_channel_free(channel);
        return nullptr;
    }

    rc = ssh_channel_request_pty(channel);
    if (rc != SSH_OK) {
        std::cerr << "Error requesting pty: " << ssh_get_error(session) << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return nullptr;
    }

    // todo: make the pty size dependent on window size
//    rc = ssh_channel_change_pty_size(channel, 80, 24);
//    if (rc != SSH_OK) {
//        std::cerr << "Error changing pty size: " << ssh_get_error(session) << std::endl;
//        ssh_channel_close(channel);
//        ssh_channel_free(channel);
//        return nullptr;
//    }

    rc = ssh_channel_request_shell(channel);
    if (rc != SSH_OK) {
        std::cerr << "Error requesting shellChannel: " << ssh_get_error(session) << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return nullptr;
    }

    return channel;
}

void RemoteShell::disconnect() {
    if (session == nullptr) {
        return;
    }

    ssh_disconnect(session);
    ssh_free(session);
    session = nullptr;
}

std::string RemoteShell::executeCommand(const std::string& command) {
    ssh_channel channel = ssh_channel_new(session);
    if (channel == nullptr) {
        std::cerr << "Error creating channel: " << ssh_get_error(session) << std::endl;
        return "";
    }

    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        std::cerr << "Error opening channel: " << ssh_get_error(session) << std::endl;
        ssh_channel_free(channel);
        return "";
    }

    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
        std::cerr << "Error executing command: " << ssh_get_error(session) << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return "";
    }

    std::string output;
    char buffer[256];
    int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);

    while (nbytes > 0) {
        if (write(1, buffer, nbytes) != (unsigned int) nbytes) {
            std::cerr << "Error reading output" << std::endl;
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return "";
        }

        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0) {
        std::cerr << "Error reading output" << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return "";
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return output;
}