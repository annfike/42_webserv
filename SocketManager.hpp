#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <fcntl.h>

class SocketManager {
public:
	SocketManager();
	SocketManager(const SocketManager& s);
	~SocketManager();
	SocketManager& operator=(const SocketManager& s);
	
	void bindSocket(int port);
	int getServerFd();
	bool isActive(fd_set& fdread, fd_set& fdwrite);
	bool acceptConnection(int socket);
	int getMax_fd();
	void close(int fd);
	bool isSocket(int fd);

private:
	fd_set master_set;  // Множество файловых дескрипторов
	//fd_set read_fds;    // Множество файловых дескрипторов, готовых для чтения
	//fd_set write_fds;
	int max_fd;         // Максимальный дескриптор для select

	std::vector<int> sockets;
};

#endif // SOCKETMANAGER_HPP

