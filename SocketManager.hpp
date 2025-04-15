#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include "Connection.hpp"
#include "ServerConfig.hpp"
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
#include <deque>
#include <fcntl.h>
#include <algorithm>
#include <sys/types.h>
#include <netdb.h>

class SocketManager {
	public:
		SocketManager();
		SocketManager(const SocketManager& s);
		~SocketManager();
		SocketManager& operator=(const SocketManager& s);
		
		void bindSocket(const ServerConfig& config);
		std::vector<Connection*> getActiveConnections();
		void acceptConnection(Connection& socket);
		Connection* getConnection(int fd);
		void closeSockets();
		void print();
		void closeConnection(Connection& con);
		std::deque<Connection> connections;
};

#endif

