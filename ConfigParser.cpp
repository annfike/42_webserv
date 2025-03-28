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
    std::ifstream file(filename.c_str());
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

        if (line.size() == 0 || line[0] == '#') continue;  // Skip empty lines or comments

        //std::cout << "Processing line: " << line << std::endl;  // Debug: show current line

        if (line.find("server {") != std::string::npos) {
            if (inside_server_block) {
                std::cerr << "Unexpected 'server {' found, already inside a server block!" << std::endl;
            }
            inside_server_block = true;
            //std::cout<<"-----------------------HERE-----------------------------------" << std::endl;
            current_server = ServerConfig();  // Reset the current server configuration
            //std::cout << "Entered server block." << std::endl;  // Debug
            continue;
        }

        if (line.find("}") != std::string::npos) {
            if (inside_location_block) {
                current_server.locations[current_location.path] = current_location;
                current_location = ServerConfig::Location();
                inside_location_block = false;
                continue;
            }

            if (!inside_server_block) {
                std::cerr << "Unexpected '}' found without an open server block!" << std::endl;
                return false;
            }

            if (current_server.listen.size() > 0) {
                servers.push_back(current_server);
                //std::cout << "Saved server with listen: " << current_server.listen << std::endl;  // Debug
            }

            inside_server_block = false;
            current_location = ServerConfig::Location();
            continue;
        }

        if (inside_server_block) {
            if (line.find("listen") != std::string::npos) {
                size_t start = line.find("listen") + 6;
                size_t colon_pos = line.find(":", start);
                if (start != std::string::npos && colon_pos != std::string::npos) {
                    std::string ip_address = line.substr(start, colon_pos - start);
                    std::string port = line.substr(colon_pos + 1);
                    trim(ip_address);
                    trim(port);
                    current_server.listen_IP = ip_address;
                    current_server.listen = port;
                }
                //std::cout << "Parsed listen: " << current_server.listen << std::endl;  // Debug
            } else if (line.find("error_page") != std::string::npos) {
                size_t start = line.find(" ");
                size_t end = line.find(";");
                if (start != std::string::npos && end != std::string::npos) {
                    std::string err = line.substr(start + 1, end - start - 1);
                    trim(err);
                    int status_code;
                    std::string path;
                    std::istringstream iss(err);
                    iss >> status_code >> path;
                    current_server.error_pages[status_code] = path;
                   
                    //std::cout << "Parsed error_page: " << current_server.error_page << std::endl;  // Debug
                }
            } else if (line.find("server_name") != std::string::npos) {
                size_t start = line.find(" ");
                size_t end = line.find(";");
                if (start != std::string::npos && end != std::string::npos) {
                    current_server.server_name = line.substr(start + 1, end - start - 1);
                    trim(current_server.server_name);
                    //std::cout << "Parsed server_name: " << current_server.error_page << std::endl;  // Debug
                }
            } else if (line.find("client_max_body_size") != std::string::npos) {
                size_t start = line.find(" ");
                size_t end = line.find(";");
                if (start != std::string::npos && end != std::string::npos) {
                    current_server.client_max_body_size = line.substr(start + 1, end - start - 1);
                    trim(current_server.client_max_body_size);
                    //std::cout << "Parsed client_max_body_size: " << current_server.client_max_body_size << std::endl;  // Debug
                }
            } else if (line.find("location") != std::string::npos) {
                if (current_location.path.size() > 0) {
                    current_server.locations[current_location.path] = current_location;
                    //std::cout << "Saved location before new one: " << current_location.path << std::endl;  // Debug
                }

                current_location = ServerConfig::Location();
                size_t path_pos = line.find("/");
                if (path_pos != std::string::npos) {
                    size_t end = line.find("{");
                    if (end == std::string::npos)
                        current_location.path = line.substr(path_pos);
                    else
                        current_location.path = line.substr(path_pos, end - path_pos);

                    trim(current_location.path);
                    //std::cout << "Entered location block with path: " << current_location.path << std::endl;  // Debug
                }

                inside_location_block = true;
            }

            if (inside_location_block) {
                if (line.find("root") != std::string::npos) {
                    size_t start = line.find_first_of("/");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        current_location.root = line.substr(start, end - start);
                        trim(current_location.root);
                        //std::cout << "Parsed root: " << current_location.root << std::endl;  // Debug
                    }
                } else if (line.find("methods") != std::string::npos) {
                    size_t start = line.find_first_of(" \t"); // Находим первый пробел или табуляцию после "methods"
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                            std::string methods_str = line.substr(start, end - start);
                            trim(methods_str); 
                            std::istringstream iss(methods_str);
                            std::string method;
                            current_location.methods.clear();
                            while (iss >> method) {
                                // Удаляем запят в конце строки, если она есть
                                if (!method.empty() && method[method.size() - 1] == ',') {
                                    method.erase(method.size() - 1);
                                }
                                current_location.methods.push_back(method);
                            }
                            /*std::cout << "!! Parsed methods: ";
                            for (size_t i = 0; i < current_location.methods.size(); ++i) {
                                std::cout << current_location.methods[i];
                                if (i < current_location.methods.size() - 1) {
                                    std::cout << ", ";
                                }
                            }
                            std::cout << std::endl; */ // Debug
                    }
                    
                } else if (line.find("autoindex") != std::string::npos) {
                    size_t start = line.find_first_of(" \t");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        std::string value = line.substr(start, end - start);
                        trim(value);
                        current_location.autoindex = (value == "on");
                        //std::cout << "Parsed autoindex: " << current_location.autoindex << std::endl;  // Debug
                    }
                } else if (line.find("index") != std::string::npos) {
                    size_t start = line.find_first_of(" \t");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        current_location.index = line.substr(start, end - start);
                        trim(current_location.index);
                        //std::cout << "Parsed index: " << current_location.index << std::endl;  // Debug
                    }
                } else if (line.find("max_body") != std::string::npos) {
                    size_t start = line.find_first_of(" \t");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        current_location.max_body = line.substr(start, end - start);
                        trim(current_location.max_body);
                        //std::cout << "Parsed max_body: " << current_location.max_body << std::endl;  // Debug
                    }
                }
                else if (line.find("upload_store") != std::string::npos) {
                    size_t start = line.find_first_of(" \t");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        current_location.upload_store = line.substr(start, end - start);
                        trim(current_location.upload_store);
                        //std::cout << "Parsed upload_store: " << current_location.upload_store << std::endl;  // Debug
                    }
                }
                else if (line.find("exec") != std::string::npos) {
                    size_t start = line.find_first_of(" \t");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        current_location.exec = line.substr(start, end - start);
                        trim(current_location.exec);
                        //std::cout << "Parsed exec: " << current_location.exec << std::endl;  // Debug
                    }
                }
                else if (line.find("return") != std::string::npos) {
                    size_t start = line.find_first_of(" \t");
                    size_t end = line.find(";");
                    if (start != std::string::npos && end != std::string::npos) {
                        current_location.redirect = line.substr(start, end - start);
                        trim(current_location.redirect);
                        //std::cout << "Parsed redirect: " << current_location.redirect << std::endl;  // Debug
                    }
                }
                

                if (line.find("}") != std::string::npos) {
                    if (inside_location_block) {
                        current_server.locations[current_location.path] = current_location;
                        current_location = ServerConfig::Location();
                        //std::cout << "Saved location and reset." << std::endl;  // Debug
                        inside_location_block = false;
                    }
                }
            }
        }
    }

    if (inside_server_block) {
        if (current_location.path.size() > 0) {
            current_server.locations[current_location.path] = current_location;
            //std::cout << "Saved last location: " << current_location.path << std::endl;  // Debug
        }
        if (current_server.listen.size() > 0) {
            servers.push_back(current_server);
            //std::cout << "Saved last server with listen: " << current_server.listen << std::endl;  // Debug
        }
    }

    if (inside_server_block) {
        std::cerr << "Error: Missing '}' to close the server block." << std::endl;
        return false;
    }

    file.close();
    return true;
}



