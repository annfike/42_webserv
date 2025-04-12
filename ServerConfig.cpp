#include "ServerConfig.hpp"

void ServerConfig::Location::print() const {
    std::cout << "	_Path: " << path;
    //std::cout << "  Methods: " << methods << std::endl;
    std::cout << "	_Methods: ";
    for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
        std::cout << *it << " ";
    }
    
    std::cout << "	_Index: " << index;
    std::cout << "	_Root: " << root;
    std::cout << "	_Exec: " << exec;
    std::cout << "	_Upload_Store: " << upload_store;
    std::cout << "	_Autoindex: " << (autoindex ? "on" : "off");
    std::cout << "	_Max_Body: " << max_body;
    std::cout << "	_Redirect: " << redirect << std::endl;
}

void ServerConfig::print() const {
    std::cout << "listen: " << listen << std::endl;
    std::cout << "listen_IP: " << listen_IP << std::endl;
    std::cout << "error_pages: " << error_pages.size() << std::endl;
    for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }

    std::cout << "server_name: " << server_name << std::endl;
    std::cout << "client_max_body_size: " << client_max_body_size << std::endl;
    for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        std::cout << "location: " << it->first << std::endl;
        it->second.print();
    }
}

const std::vector<std::string>& ServerConfig::getCgiExtension() const
{
	if (!locations.empty()) {
        return locations.begin()->second.cgi_extension;
    }

    static const std::vector<std::string> emptyVec;
    return emptyVec;
}

const std::string& ServerConfig::getRootLocation(const std::string& locationKey) const {
    std::map<std::string, Location>::const_iterator it = locations.find(locationKey);
    if (it != locations.end()) {
        return it->second.root;
    }
    static const std::string emptyString;
    return emptyString;
}

/*const ServerConfig::Location* getLocationByIndex(const ServerConfig& config, int index) {
    int i = 0;
    for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
         it != config.locations.end(); ++it, ++i) {
        if (i == index) {
            return &(it->second);
        }
    }
    return NULL;
}*/
