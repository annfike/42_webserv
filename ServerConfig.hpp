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
        size_t max_body;
        std::string redirect;
        std::string cgiPath;
        std::vector<std::string> cgi_extension;
        bool autoindex;

        Location() :  path(""), index(""), root(""), exec(""), upload_store(""), max_body(0), 
            redirect(""), cgiPath(""), autoindex(true) {}
        Location(const Location& l) : extension_path(l.extension_path), path(l.path), index(l.index), root(l.root), exec(l.exec), upload_store(l.upload_store), 
            max_body(l.max_body), redirect(l.redirect), cgiPath(l.cgiPath), cgi_extension(l.cgi_extension), autoindex(l.autoindex) {
                for (size_t i = 0; i < l.methods.size(); i++)
                {
                    methods.push_back(l.methods[i]);
                }  
            }
        Location &operator=(const Location& l){
            if (this != &l)
            {
                extension_path = l.extension_path;
                path = l.path;
                methods.clear();
                for (size_t i = 0; i < l.methods.size(); i++)
                {
                    methods.push_back(l.methods[i]);
                }               
                index = l.index;
                root = l.root;
                exec = l.exec;
                upload_store = l.upload_store;
                max_body = l.max_body;
                redirect = l.redirect;
                cgiPath = l.cgiPath;
                cgi_extension = l.cgi_extension;
                autoindex = l.autoindex;
            }
            return *this;}
        void print() const;
    };

    std::string listen;
    std::string listen_IP;
    std::map<int, std::string> error_pages; //code-path
    std::string server_name;
    size_t client_max_body_size;
    std::map<std::string, Location> locations;

    ServerConfig() :  listen(""), listen_IP(""), server_name(""), client_max_body_size(-1) {}
    ServerConfig(const ServerConfig& s) : listen(s.listen), listen_IP(s.listen_IP), error_pages(s.error_pages), 
        server_name(s.server_name), client_max_body_size(s.client_max_body_size), locations(s.locations){ }
        ServerConfig& operator=(const ServerConfig& l) {
            if (this == &l) {
               return *this;
            }        

            listen = l.listen;
            listen_IP = l.listen_IP;
            server_name = l.server_name;
            client_max_body_size = l.client_max_body_size;

            locations = l.locations;
        
            return *this;
        }
    void print() const;
    const std::vector<std::string>& getCgiExtension() const;
    const std::string& getRootLocation(const std::string& locationKey) const;
};

#endif