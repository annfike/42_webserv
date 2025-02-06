#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <fcntl.h>
#include <algorithm>

class SocketManager {
public:
	SocketManager();
	SocketManager(const SocketManager& s);
	~SocketManager();
	SocketManager& operator=(const SocketManager& s);
	
	void bindSocket(int port);
	int getServerFd();
	bool getActive(std::vector<struct pollfd>& fds);
	struct pollfd acceptConnection(struct pollfd socket);
	void closeFd(int fd);
	bool isSocket(int fd);
	void closeSockets();
	std::vector<struct pollfd> sockets;

private:
	//std::vector<struct pollfd> sockets;
	std::vector<int> ports;
};

#endif

