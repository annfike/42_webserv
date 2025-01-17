#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class SocketManager {
public:
    SocketManager();
    ~SocketManager();

    void bindAndListen(int port);
    int getServerFd();

private:
    int server_fd;
    void createSocket();
    void bindSocket(int port);
    void listenSocket();
    
};

#endif // SOCKETMANAGER_HPP

