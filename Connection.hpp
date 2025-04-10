#pragma once

#include <poll.h>
#include <string>
#include "ServerConfig.hpp"
#include <fstream>

class Connection
{
	private:
		
	public:
		Connection(): headerSent(false), keepAlive(false){ file = new std::ifstream(); }
		~Connection(){ delete file; }
		std::vector<ServerConfig> configs;
		struct pollfd poll;
		
		bool isSocket;
		std::string ip;
		int port;
		std::ifstream* file;
		bool headerSent;
		bool keepAlive;
		std::string responseHeader;

		ServerConfig& getConfig(std::string serverName);
};