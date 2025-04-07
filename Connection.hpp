#ifndef pragma

#include <poll.h>
#include <string>
#include "ServerConfig.hpp"

class Connection
{
	private:

	public:
		Connection(): headerSent(false), keepAlive(false){}

		std::vector<ServerConfig> configs;
		struct pollfd poll;
		bool isSocket;
		std::string ip;
		int port;
		bool headerSent;
		bool keepAlive;

		ServerConfig& getConfig(std::string serverName);
};

#endif