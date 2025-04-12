#pragma once

#include <poll.h>
#include <string>
#include "ServerConfig.hpp"
#include <fstream>

class Connection
{
	private:
		
	public:
		Connection():transferred(0){}
		std::vector<ServerConfig> configs;
		struct pollfd poll;
		bool isSocket;
		std::string ip;
		int port;
		bool headerSent;
		bool keepAlive;
		std::string responseHeader;
		std::size_t transferred;

		ServerConfig& getConfig(std::string serverName);
};