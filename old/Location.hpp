#include "Server.hpp"

// Класс для хранения настроек Location
class Location {
public:
    std::string path;
    std::string methods;
    std::string index;
    std::string root;
    std::string exec;
    std::string upload_store;
    bool autoindex;

    Location() : autoindex(false) {}

    void print() const {
        std::cout << "  Path: " << path << std::endl;
        std::cout << "  Methods: " << methods << std::endl;
        std::cout << "  Index: " << index << std::endl;
        std::cout << "  Root: " << root << std::endl;
        std::cout << "  Exec: " << exec << std::endl;
        std::cout << "  Upload Store: " << upload_store << std::endl;
        std::cout << "  Autoindex: " << (autoindex ? "on" : "off") << std::endl;
    }
};