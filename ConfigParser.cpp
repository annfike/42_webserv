#include "ConfigParser.hpp"
#include <fstream>
#include <algorithm>
#include <cctype>


void ConfigParser::trim(std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");

    if (first != std::string::npos && last != std::string::npos) str = str.substr(first, last - first + 1);
    else str.clear();
}


bool ConfigParser::parseConfig(const std::string &filename, std::vector<ServerConfig> &servers) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return false;
    }

    std::string line;
    ServerConfig current_server;
    ServerConfig::Location current_location;
    bool inside_server_block = false;
    bool inside_location_block = false;

    while (std::getline(file, line)) {
        trim(line);

        if (line.empty() || line[0] == '#') continue;  // Skip empty lines or comments

        std::cout << "Processing line: " << line << std::endl;  // Debug: show current line

        // Start of a new server block
        if (line.find("server {") != std::string::npos) {
            if (inside_server_block) {
                std::cerr << "Unexpected 'server {' found, already inside a server block!" << std::endl;
                return false;
            }
            inside_server_block = true;
            current_server = ServerConfig();  // Reset the current server configuration
            std::cout << "Entered server block." << std::endl;  // Debug
            continue;
        }

        // End of the current server block
        if (line.find("}") != std::string::npos) {
            if (inside_location_block) {
                // This closing brace is for the location block, not the server block
                std::cout << "Exiting location block: " << current_location.path << std::endl;  // Debug
                current_server.locations[current_location.path] = current_location;
                current_location = ServerConfig::Location();  // Reset location
                inside_location_block = false;
                continue;  // Skip the remaining part of this iteration and continue to the next line
            }

            if (!inside_server_block) {
                std::cerr << "Unexpected '}' found without an open server block!" << std::endl;
                return false;
            }

            // Save the current location if it's not empty
            if (!current_location.path.empty()) {
                current_server.locations[current_location.path] = current_location;
                std::cout << "Saved location: " << current_location.path << std::endl;  // Debug
            }

            // Push the current server if it has a valid 'listen' directive
            if (!current_server.listen.empty()) {
                servers.push_back(current_server);
                std::cout << "Saved server with listen: " << current_server.listen << std::endl;  // Debug
            }

            // Reset for the next server
            inside_server_block = false;
            current_location = ServerConfig::Location();
            continue;
        }

        // Parse server-level directives inside the server block
        if (inside_server_block) {
            if (line.find("listen") != std::string::npos) {
                size_t pos = line.find(":");
                current_server.listen = line.substr(pos + 1);
                trim(current_server.listen);
                std::cout << "Parsed listen: " << current_server.listen << std::endl;  // Debug
            } else if (line.find("error_page") != std::string::npos) {
                size_t pos = line.find(" ");  // Find the first space after "error_page"
                if (pos != std::string::npos) {
                    // Extract the value after the space
                    current_server.error_page = line.substr(pos + 1);  // Everything after the first space
                    trim(current_server.error_page);  // Remove leading/trailing spaces
                    std::cout << "Parsed error_page: " << current_server.error_page << std::endl;  // Debug
                }
            } else if (line.find("server_name") != std::string::npos) {
                size_t pos = line.find(" ");  // Find the first space after "server_name"
                if (pos != std::string::npos) {
                    // Extract the value after the space
                    current_server.server_name = line.substr(pos + 1);  // Everything after the first space
                    trim(current_server.server_name);  // Remove leading/trailing spaces
                    std::cout << "Parsed server_name: " << current_server.server_name << std::endl;  // Debug
                }
            } else if (line.find("client_max_body_size") != std::string::npos) {
                size_t pos = line.find(" ");
                current_server.client_max_body_size = line.substr(pos + 1);
                trim(current_server.client_max_body_size);
                std::cout << "Parsed client_max_body_size: " << current_server.client_max_body_size << std::endl;  // Debug
            } else if (line.find("location") != std::string::npos) {
                // Save the current location before starting a new one
                if (!current_location.path.empty()) {
                    current_server.locations[current_location.path] = current_location;
                    std::cout << "Saved location before new one: " << current_location.path << std::endl;  // Debug
                }

                // Reset the current location and parse the path
                current_location = ServerConfig::Location();
                size_t path_pos = line.find("/");
                if (path_pos != std::string::npos) {
                    current_location.path = line.substr(path_pos);
                    trim(current_location.path);
                    std::cout << "Entered location block with path: " << current_location.path << std::endl;  // Debug
                }

                inside_location_block = true;  // Indicate that we are inside a location block
            }

            // Parse location-level directives inside the location block
            if (inside_location_block) {
                if (line.find("root") != std::string::npos) {
                    size_t pos = line.find(":");
                    current_location.root = line.substr(pos + 1);
                    trim(current_location.root);
                    std::cout << "Parsed root: " << current_location.root << std::endl;  // Debug
                } else if (line.find("methods") != std::string::npos) {
                    size_t pos = line.find(":");
                    current_location.methods = line.substr(pos + 1);
                    trim(current_location.methods);
                    std::cout << "Parsed methods: " << current_location.methods << std::endl;  // Debug
                } else if (line.find("autoindex") != std::string::npos) {
                    size_t pos = line.find(":");
                    std::string value = line.substr(pos + 1);
                    trim(value);
                    current_location.autoindex = (value == "on");
                    std::cout << "Parsed autoindex: " << current_location.autoindex << std::endl;  // Debug
                } else if (line.find("index") != std::string::npos) {
                    size_t pos = line.find(":");
                    current_location.index = line.substr(pos + 1);
                    trim(current_location.index);
                    std::cout << "Parsed index: " << current_location.index << std::endl;  // Debug
                } else if (line.find("max_body") != std::string::npos) {
                    size_t pos = line.find(":");
                    std::string value = line.substr(pos + 1);
                    trim(value);
                    current_location.max_body = value;
                    std::cout << "Parsed max_body: " << current_location.max_body << std::endl;  // Debug
                }

                // If a location block is complete (i.e., we've parsed all directives), reset the flag
                if (line.find("}") != std::string::npos) {
                    if (inside_location_block) {
                        current_server.locations[current_location.path] = current_location;
                        current_location = ServerConfig::Location();  // Reset location
                        std::cout << "Saved location and reset." << std::endl;  // Debug
                        inside_location_block = false;  // Exit the location block
                    }
                }
            }
        }
    }

    // Final save for the last location and server (if not already saved)
    if (inside_server_block) {
        if (!current_location.path.empty()) {
            current_server.locations[current_location.path] = current_location;
            std::cout << "Saved last location: " << current_location.path << std::endl;  // Debug
        }
        if (!current_server.listen.empty()) {
            servers.push_back(current_server);
            std::cout << "Saved last server with listen: " << current_server.listen << std::endl;  // Debug
        }
    }

    // Check for any mismatched brackets
    if (inside_server_block) {
        std::cerr << "Error: Missing '}' to close the server block." << std::endl;
        return false;
    }

    file.close();
    return true;
}
