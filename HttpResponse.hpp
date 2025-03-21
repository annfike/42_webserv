#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include "ServerConfig.hpp"
#include <fstream>

class Response {
public:
    enum Type {
        ERROR,
        REDIRECT,
        FOLDER_LIST,
        FILE
    };

    Response(Type type, int code = 0, const std::string& message = "", const std::string& destination = "", const std::string& filePath = "");
    void print() const;
    Response handleRequest(const ServerConfig& config, const std::string& method, const std::string& url, size_t bodySize);
    const char* toHttpResponse() const;
    std::string getPath(const ServerConfig& config, const std::string& url);
    int getLocation(const ServerConfig& config, const std::string& url);
    std::string findLocalPath(const ServerConfig& config, const std::string& url, int location_index);

private:
    Type type;
    int code;
    std::string message;
    std::string destination;
    std::string filePath;
    std::string urlLocal;
};

#endif // HTTPRESPONSE_HPP