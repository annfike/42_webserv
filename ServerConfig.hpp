#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <iostream>
#include <vector>

// будет содержать настройки сервера
class ServerConfig {
public:
    class Location {
    public:
        std::map<std::string, std::string> extension_path;

        std::string path;
        std::vector<std::string> methods;
        std::string index;
        std::string root;
        std::string exec;
        std::string upload_store;
        std::string max_body;
        std::string redirect;
        std::string cgiPath;
        std::vector<std::string> cgi_extension;
        bool autoindex;

        Location() : autoindex(true) {}

        void print() const;
    };

    std::string listen;
    std::string listen_IP;
    std::map<int, std::string> error_pages; //code-path
    std::string server_name;
    size_t client_max_body_size;
    std::map<std::string, Location> locations;

    void print() const;
    const std::vector<std::string>& getCgiExtension() const;
    const std::string& getRootLocation(const std::string& locationKey) const;

    //const Location* getLocationByIndex(const ServerConfig& config, int index);
};

#endif // SERVERCONFIG_HPP
