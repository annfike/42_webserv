#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib>

class HttpRequestParser {
private:
    std::string method;
    std::string url;
    std::string path;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    bool multipart;
    std::string query;

public:
    HttpRequestParser();
    void parse(const char* buffer);
    void printRequest() const;

    const std::string& getMethod() const;
    const std::string& getUrl() const;
    const std::string& getBody() const;
    const std::string& getHeader(const std::string& key) const;
    std::map<std::string, std::string>& getHeaders();
    std::string& getQuery();
    std::string& getPath();

    std::string hostName;
};

#endif // HTTPREQUEST_HPP