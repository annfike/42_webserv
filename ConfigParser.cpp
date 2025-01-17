#include "ConfigParser.hpp"
#include <fstream>
#include <algorithm>
#include <cctype>

bool ConfigParser::parseConfig(const std::string &filename, std::vector<ServerConfig> &servers) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return false;
    }

    std::string line;
    ServerConfig current_server;
    ServerConfig::Location current_location;

    while (std::getline(file, line)) {
        trim(line);

        if (line.empty() || line[0] == '#') continue;

        if (line.find("server {") != std::string::npos) {
            current_server = ServerConfig();
            continue;
        }

        if (line.find("}") != std::string::npos) {
            if (!current_server.listen.empty()) servers.push_back(current_server);
            continue;
        }

        if (line.find("listen") != std::string::npos) {
            size_t pos = line.find(":");
            current_server.listen = line.substr(pos + 1);
            trim(current_server.listen);
        } else if (line.find("error_page") != std::string::npos) {
            size_t pos = line.find(":");
            current_server.error_page = line.substr(pos + 1);
            trim(current_server.error_page);
        } else if (line.find("location") != std::string::npos) {
            if (!current_location.path.empty()) current_server.locations[current_location.path] = current_location;
            current_location = ServerConfig::Location();
            size_t path_pos = line.find("/");
            if (path_pos != std::string::npos) current_location.path = line.substr(path_pos);
            trim(current_location.path);
        } else if (line.find("methods") != std::string::npos) {
            size_t pos = line.find(":");
            current_location.methods = line.substr(pos + 1);
            trim(current_location.methods);
        } else if (line.find("index") != std::string::npos) {
            size_t pos = line.find(":");
            current_location.index = line.substr(pos + 1);
            trim(current_location.index);
        } else if (line.find("root") != std::string::npos) {
            size_t pos = line.find(":");
            current_location.root = line.substr(pos + 1);
            trim(current_location.root);
        } else if (line.find("exec") != std::string::npos) {
            size_t pos = line.find(":");
            current_location.exec = line.substr(pos + 1);
            trim(current_location.exec);
        } else if (line.find("upload_store") != std::string::npos) {
            size_t pos = line.find(":");
            current_location.upload_store = line.substr(pos + 1);
            trim(current_location.upload_store);
        } else if (line.find("autoindex") != std::string::npos) {
            size_t pos = line.find(":");
            std::string value = line.substr(pos + 1);
            trim(value);
            current_location.autoindex = (value == "on");
        }
    }

    if (!current_location.path.empty()) current_server.locations[current_location.path] = current_location;
    if (!current_server.listen.empty()) servers.push_back(current_server);

    file.close();
    return true;
}

void ConfigParser::trim(std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");

    if (first != std::string::npos && last != std::string::npos) str = str.substr(first, last - first + 1);
    else str.clear();
}
