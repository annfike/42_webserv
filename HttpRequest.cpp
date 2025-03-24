#include "HttpRequest.hpp"

HttpRequestParser::HttpRequestParser() {}

std::string decodeChunkedBody(std::istringstream &request)
{
    std::string decodedBody;
    std::string line;
    
    while (std::getline(request, line)) {
        // Remove carriage return (\r) if present
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Convert hex chunk size to integer
        std::istringstream hexStream(line);
        int chunkSize;
        hexStream >> std::hex >> chunkSize;

        // Stop if the last chunk (0) is reached
        if (chunkSize == 0)
            break;

        // Read the chunk data
        char *buffer = new char[chunkSize + 1];
        request.read(buffer, chunkSize);
        buffer[chunkSize] = '\0';  // Null-terminate

        decodedBody.append(buffer, chunkSize);
        delete[] buffer;

        // Read and discard the trailing \r\n
        std::getline(request, line);
    }

    return decodedBody;
}

void HttpRequestParser::parse(const char* buffer) 
{
        /*buffer = "HTTP/1.1 200 OK\r\n\
Content-Type: text/plain\r\n\
Transfer-Encoding: chunked\r\n\
Connection: keep-alive\r\n\
\r\n\
9\r\n\
chunk 1, \r\n\
7\r\n\
chunk 2\r\n\
0\r\n\
\r\n";*/

    std::istringstream request(buffer);
    std::string line;

    // Parse the request line
    std::getline(request, line);
    std::istringstream requestLine(line);
    requestLine >> method >> url >> httpVersion;

    // Parse headers
    while (std::getline(request, line)) {
        if (line.empty() || line == "\r")
            break; // End of headers
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2, line.size() - colonPos - 3); // Skip ": "
            headers[key] = value;
        }
    }

    // Parse body if present
    std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
    std::map<std::string, std::string>::const_iterator chunked = headers.find("Transfer-Encoding");
    if (it != headers.end()) 
    {
        size_t contentLength = std::strtoul(it->second.c_str(), NULL, 10); // Аналог std::stoul в C++98
        body.resize(contentLength);
        request.read(&body[0], contentLength);        
    }
    else if (chunked != headers.end() && chunked->second == "chunked")
    {
        body = decodeChunkedBody(request);
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