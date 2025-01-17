#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

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

// Класс для хранения настроек сервера
class ServerConfig {
public:
    std::string listen;
    std::string error_page;
    std::map<std::string, Location> locations;

    void print() const {
        std::cout << "Listen: " << listen << std::endl;
        std::cout << "Error Page: " << error_page << std::endl;
        std::cout << "Locations: " << std::endl;
        // Используем обычный цикл для обхода map
        for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
            std::cout << "Location " << it->first << ":" << std::endl;
            it->second.print();
        }
    }
};

// Класс для парсинга конфигурационного файла
class ConfigParser {
public:
    static bool parseConfig(const std::string &filename, std::vector<ServerConfig> &servers) {
        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            std::cerr << "Не удалось открыть файл: " << filename << std::endl;
            return false;
        }

        std::string line;
        ServerConfig current_server;
        Location current_location;

        while (std::getline(file, line)) {
            trim(line);  // Убираем лишние пробелы и символы новой строки

            if (line.empty() || line[0] == '#') {
                continue;  // Пропускаем пустые строки и комментарии
            }

            // Начало блока server
            if (line.find("server {") != std::string::npos) {
                current_server = ServerConfig();
                continue;
            }

            // Конец блока server
            if (line.find("}") != std::string::npos) {
                if (!current_server.listen.empty()) {
                    servers.push_back(current_server);
                }
                continue;
            }

            // Заполняем параметры сервера
            if (line.find("listen") != std::string::npos) {
                size_t pos = line.find(":");
                current_server.listen = line.substr(pos + 1);
                trim(current_server.listen);
            }
            else if (line.find("error_page") != std::string::npos) {
                size_t pos = line.find(":");
                current_server.error_page = line.substr(pos + 1);
                trim(current_server.error_page);
            }
            // Парсим блоки location
            else if (line.find("location") != std::string::npos) {
                // Добавляем предыдущее location
                if (!current_location.path.empty()) {
                    current_server.locations[current_location.path] = current_location;
                }
                current_location = Location();  // Сбрасываем настройки location
                size_t path_pos = line.find("/");
                if (path_pos != std::string::npos) {
                    current_location.path = line.substr(path_pos);
                    trim(current_location.path);
                }
            }
            // Парсим параметры внутри блока location
            else if (line.find("methods") != std::string::npos) {
                size_t pos = line.find(":");
                current_location.methods = line.substr(pos + 1);
                trim(current_location.methods);
            }
            else if (line.find("index") != std::string::npos) {
                size_t pos = line.find(":");
                current_location.index = line.substr(pos + 1);
                trim(current_location.index);
            }
            else if (line.find("root") != std::string::npos) {
                size_t pos = line.find(":");
                current_location.root = line.substr(pos + 1);
                trim(current_location.root);
            }
            else if (line.find("exec") != std::string::npos) {
                size_t pos = line.find(":");
                current_location.exec = line.substr(pos + 1);
                trim(current_location.exec);
            }
            else if (line.find("upload_store") != std::string::npos) {
                size_t pos = line.find(":");
                current_location.upload_store = line.substr(pos + 1);
                trim(current_location.upload_store);
            }
            else if (line.find("autoindex") != std::string::npos) {
                size_t pos = line.find(":");
                std::string value = line.substr(pos + 1);
                trim(value);
                current_location.autoindex = (value == "on");
            }
        }

        // Добавляем последнее location
        if (!current_location.path.empty()) {
            current_server.locations[current_location.path] = current_location;
        }

        // Добавляем последний сервер
        if (!current_server.listen.empty()) {
            servers.push_back(current_server);
        }

        file.close();
        return true;
    }

private:
    // Убираем пробелы в начале и в конце строки
    static void trim(std::string &str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        size_t last = str.find_last_not_of(" \t\r\n");

        if (first != std::string::npos && last != std::string::npos) {
            str = str.substr(first, last - first + 1);
        } else {
            str.clear();
        }
    }
};

int main() {
    std::vector<ServerConfig> servers;

    // Парсим конфигурационный файл
    if (ConfigParser::parseConfig("config.config", servers)) {
        // Выводим полученные данные
        for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
            it->print();
        }
    }

    return 0;
}

/*
c++ -o parse parse_config.cpp
*/