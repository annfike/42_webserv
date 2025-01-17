#include "SocketManager.hpp"

SocketManager::SocketManager() {
    createSocket();
}

SocketManager::~SocketManager() {
    close(server_fd);
}

void SocketManager::createSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Ошибка при создании сокета");
    }
}

void SocketManager::bindAndListen(int port) {
    bindSocket(port);
    listenSocket();
}

void SocketManager::bindSocket(int port) {
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        throw std::runtime_error("Ошибка при привязке сокета к адресу");
    }
}

void SocketManager::listenSocket() {
    if (listen(server_fd, 10) == -1) {
        throw std::runtime_error("Ошибка при переводе сокета в режим прослушивания");
    }
}

int SocketManager::getServerFd() {
    return server_fd;
}