#include "HttpRequest.hpp"

HttpRequestParser::HttpRequestParser() {}

void HttpRequestParser::parse(const char* buffer) {
    std::istringstream request(buffer);
    std::string line;

    // Parse the request line
    std::getline(request, line);
    std::istringstream requestLine(line);
    requestLine >> method >> url >> httpVersion;

    // Parse headers
    while (std::getline(request, line)) {
        if (line.empty() || line == "\r") break; // End of headers
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2); // Skip ": "
            headers[key] = value;
        }
    }

    // Parse body if present
    std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
    if (it != headers.end()) {
        size_t contentLength = std::strtoul(it->second.c_str(), NULL, 10); // Аналог std::stoul в C++98
        body.resize(contentLength);
        request.read(&body[0], contentLength);
    }

    it = headers.find("Host");
    hostName = "";
    if (it != headers.end()) {
        std::string host = it->second;
        std::size_t pos = host.find(':');
        if (pos != std::string::npos)
        {
            hostName = host.substr(0, pos);
        }
    }
}

void HttpRequestParser::printRequest() const {
    std::cout << "-------------------Request-------------------------------" << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "URL: " << url << std::endl;
    std::cout << "Host: " << hostName << std::endl;
    std::cout << "HTTP Version: " << httpVersion << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body: " << body << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
}

const std::string& HttpRequestParser::getMethod() const {
    return method;
}

const std::string& HttpRequestParser::getUrl() const {
    return url;
}

const std::string& HttpRequestParser::getBody() const {
    return body;
}