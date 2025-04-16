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
    std::vector<char> body;    

public:
    HttpRequestParser();
    void parse(const std::vector<char>& buffer, std::size_t rn);
    void parseBody(const std::vector<char>& buffer, std::size_t rn);
    bool parseChunkedBody(const std::vector<char>& buffer, std::size_t rn);
    void printRequest() const;

    const std::string& getMethod() const;
    const std::string& getUrl() const;
    const std::vector<char>& getBody() const;
    const std::string& getHeader(const std::string& key) const;
    std::map<std::string, std::string>& getHeaders();
    std::string& getPath();

    std::string httpVersion;
    std::string hostName;
    std::string boundary;
    size_t contentLength;
    bool keepAlive;
    std::string query;
    std::map<std::string, std::string> headers;
    std::size_t chunkParseOffset;
};

#endif