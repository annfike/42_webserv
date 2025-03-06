#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include "ServerConfig.hpp"

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

private:
    Type type;
    int code;
    std::string message;
    std::string destination;
    std::string filePath;
};

#endif // HTTPRESPONSE_HPP