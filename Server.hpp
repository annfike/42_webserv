#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "SocketManager.hpp"
#include <vector>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>

class Server {
public:
	Server();
	Server(const Server& s);
	Server(const std::string &config);
	~Server();
	Server*& operator=(const Server& s);

	void loop();

private:
	std::vector<ServerConfig> servers;
	SocketManager socketManager;

	void parseConfig(const std::string &config);
	void execRead(int fd, std::vector<int>& deletefds);
};

#endif // SERVER_HPP
