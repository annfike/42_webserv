#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "SocketManager.hpp"
#include <vector>

class Server {
public:
    Server(const std::string &config);
    ~Server();
    void loop();

private:
    std::vector<ServerConfig> servers;
    SocketManager socketManager;

    void parseConfig(const std::string &config);
};

#endif // SERVER_HPP
