#ifndef pragma

#include <poll.h>
#include <string>
#include "ServerConfig.hpp"

class Connection
{
	private:

	public:
		ServerConfig config;
		struct pollfd poll;
		bool isSocket;
		std::string ip;
		int port;
};

#endif