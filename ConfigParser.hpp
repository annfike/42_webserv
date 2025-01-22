#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ServerConfig.hpp"

class ConfigParser {
public:
    static bool parseConfig(const std::string &filename, std::vector<ServerConfig> &servers);

private:
    static void trim(std::string &str);
};

#endif // CONFIGPARSER_HPP
