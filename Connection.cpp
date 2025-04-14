#include "Connection.hpp"

const ServerConfig& Connection::getConfig(const std::string& serverName)
{
	for (size_t i = 0; i < configs.size(); i++)
	{
		if (configs[i].server_name == serverName)
		{
			return configs[i];
		}
	}

	for (size_t i = 0; i < configs.size(); i++)
	{
		if (configs[i].server_name.empty())
		{
			return configs[i];
		}
	}

	return configs[0];
}