#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

class HttpRequestParser {
private:
    std::string method;
    std::string url;
    std::string path;
    std::map<std::string, std::string> headers;
    std::vector<char> body;    
    std::string query;

public:
    HttpRequestParser();
    void parse(const std::vector<char>& buffer);
    void printRequest() const;

    const std::string& getMethod() const;
    const std::string& getUrl() const;
    const std::vector<char>& getBody() const;
    const std::string& getHeader(const std::string& key) const;
    std::map<std::string, std::string>& getHeaders();
    std::string& getQuery();
    std::string& getPath();

    std::string httpVersion;
    std::string hostName;
    std::string boundary;
    size_t contentLength;
    bool keepAlive;

};

#endif