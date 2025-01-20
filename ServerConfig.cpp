#include "ServerConfig.hpp"

void ServerConfig::Location::print() const {
    std::cout << "  Path: " << path << std::endl;
    std::cout << "  Methods: " << methods << std::endl;
    std::cout << "  Index: " << index << std::endl;
    std::cout << "  Root: " << root << std::endl;
    std::cout << "  Exec: " << exec << std::endl;
    std::cout << "  Upload Store: " << upload_store << std::endl;
    std::cout << "  Autoindex: " << (autoindex ? "on" : "off") << std::endl;
    std::cout << "  Max Body: " << max_body << std::endl;
}

void ServerConfig::print() const {
    std::cout << "listen: " << listen << std::endl;
    std::cout << "error_page: " << error_page << std::endl;
    std::cout << "server_name: " << server_name << std::endl;
    std::cout << "client_max_body_size: " << client_max_body_size << std::endl;
    for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        std::cout << "location: " << it->first << std::endl;
        it->second.print();
    }
}
