#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <iostream>
#include <vector>

//будет содержать настройки сервера
class ServerConfig {
public:
    class Location {
    public:
        std::string path;
        std::vector<std::string> methods;
        std::string index;
        std::string root;
        std::string exec;
        std::string upload_store;
        std::string max_body;
        std::string redirect;
        bool autoindex;

        Location() : autoindex(true) {}

        void print() const;
         
    };

    std::string listen;
    std::string listen_IP;
    std::string error_page;
    std::string server_name;
    std::string client_max_body_size;
    std::map<std::string, Location> locations;

    void print() const;
    //const Location* getLocationByIndex(const ServerConfig& config, int index);
};

#endif // SERVERCONFIG_HPP
