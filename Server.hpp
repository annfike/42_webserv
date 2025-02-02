#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "SocketManager.hpp"
#include <vector>

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
};

#endif // SERVER_HPP
