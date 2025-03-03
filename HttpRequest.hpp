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
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    HttpRequestParser();
    void parse(const char* buffer);
    void printRequest() const;

    const std::string& getMethod() const;
    const std::string& getUrl() const;
    const std::string& getBody() const;
};

#endif // HTTPREQUEST_HPP