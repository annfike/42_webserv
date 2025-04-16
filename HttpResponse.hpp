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
#include "HttpRequest.hpp"
#include <fstream>
#include <filesystem>
#include "CGIManager.hpp"
#include "Server.hpp"
#include <cmath>

class Response {
public:
    enum Type {
        ERROR,
        REDIRECT,
        FOLDER_LIST,
        FILE,
        CGI
    };

    Response(
        Type type,
        int code = 200,
        const std::string& message = "",
        const std::string& destination = "",
        const std::string& filePath = "",
        const std::string& cgi_output = ""
    );

    void              print() const;
    static Response handleRequest(const ServerConfig& config, HttpRequestParser& request);
    const std::string toHttpResponse(bool keepAlive, bool noBody) const;
    std::string cgi_output;

private:
    Type type;
    int code;
    std::string message;
    std::string destination;
    std::string filePath;
    std::string urlLocal;    
};

#endif 