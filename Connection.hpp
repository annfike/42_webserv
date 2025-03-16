#ifndef pragma

#include <poll.h>
#include <string>
#include "ServerConfig.hpp"

class Connection
{
	private:

	public:
		std::vector<ServerConfig> configs;
		struct pollfd poll;
		bool isSocket;
		std::string ip;
		int port;

		ServerConfig& getConfig(std::string serverName);
};

#endif