#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <iostream>

//будет содержать настройки сервера
class ServerConfig {
public:
    class Location {
    public:
        std::string path;
        std::string methods;
        std::string index;
        std::string root;
        std::string exec;
        std::string upload_store;
        std::string max_body;
        bool autoindex;

        Location() : autoindex(false) {}

        void print() const;
    };

    std::string listen;
    std::string error_page;
    std::string server_name;
    std::string client_max_body_size;
    std::map<std::string, Location> locations;

    void print() const;
};

#endif // SERVERCONFIG_HPP
