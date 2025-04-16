#include "HttpRequest.hpp"

HttpRequestParser::HttpRequestParser() {
    chunkParseOffset = 0;
}

void HttpRequestParser::parse(const std::vector<char>& buffer, std::size_t rn) 
{
    size_t pos = 0;
    std::string line;

    size_t lineEnd = std::find(buffer.begin(), buffer.end(), '\n') - buffer.begin();
    std::string requestLine(buffer.begin(), buffer.begin() + lineEnd);
    pos = lineEnd + 1;

    std::istringstream requestStream(requestLine);
    requestStream >> method >> url >> httpVersion;

    // Parse headers
    while (pos < rn) 
    {
        lineEnd = std::find(buffer.begin() + pos, buffer.end(), '\n') - buffer.begin();
        std::string line(buffer.begin() + pos, buffer.begin() + lineEnd);
        pos = lineEnd + 1;

        if (line == "\r" || line.empty()) 
            break; // Конец заголовков

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) 
        {                                                                                                                                                           
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2, line.size() - colonPos - 3);
            headers[key] = value;
        }
    }

    // Find the query parameters if any
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        path = url.substr(0, queryPos);        // Путь до знака вопроса
        query = url.substr(queryPos + 1);      // Все, что после знака вопроса - это query
    } else {
        path = url;  // Если query нет, то весь URL это путь
        query = "";   // Очистим query
    }

    std::map<std::string, std::string>::const_iterator it = headers.find("Host");
    hostName = "";
    if (it != headers.end()) {
        std::string host = it->second;
        std::size_t pos = host.find(':');
        if (pos != std::string::npos)
        {
            hostName = host.substr(0, pos);
        }
    }

    it = headers.find("Connection");
    keepAlive = true;
    if (it != headers.end() && it->second == "close") {        
        keepAlive = false;
    }
}

void HttpRequestParser::parseBody(const std::vector<char>& buffer, std::size_t rn)
{
    // Parse body if present
    std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
    if (it != headers.end())
    {
        std::map<std::string, std::string>::const_iterator cont = headers.find("Content-Type");
        if (cont != headers.end() && cont->second.find("multipart/form-data") != std::string::npos)
            boundary = "--" + cont->second.substr(cont->second.find("boundary=") + 9);

        contentLength = std::strtoul(it->second.c_str(), NULL, 10);
        body.resize(contentLength);
        body.assign(buffer.begin() + rn + 4, buffer.begin() + rn + 4 + contentLength);
    }
}

bool HttpRequestParser::parseChunkedBody(const std::vector<char>& buffer, std::size_t rn) 
{
    /*
    (void)buffer1;
        std::string s = "HTTP/1.1 200 OK\r\n\
Content-Type: text/plain\r\n\
Transfer-Encoding: chunked\r\n\
Connection: keep-alive\r\n\
\r\n\
9\r\n\
chunk 1, \r\n\
7\r\n\
chunk 2\r\n\
0\r\n\
\r\n";
rn = s.find("\r\n\r\n");
std::vector<char> buffer(s.begin(), s.end());*/

    if (chunkParseOffset == 0)
        chunkParseOffset = rn + 4;

    while (true) {
        // Найти конец строки с размером чанка
        std::size_t lineEnd = std::search(buffer.begin() + chunkParseOffset, buffer.end(),
                                          "\r\n", "\r\n" + 2) - buffer.begin();
        if (lineEnd == buffer.size())
            return false; // Ждём больше данных

        std::string chunkSizeStr(buffer.begin() + chunkParseOffset, buffer.begin() + lineEnd);
        std::istringstream hexStream(chunkSizeStr);
        std::size_t chunkSize = 0;
        hexStream >> std::hex >> chunkSize;
       
        chunkParseOffset = lineEnd + 2; // Пропустить \r\n

        // Конец всех чанков
        if (chunkSize == 0) {
            if (buffer.size() < chunkParseOffset + 2)
                return false;

            if (!(buffer[chunkParseOffset] == '\r' && buffer[chunkParseOffset + 1] == '\n'))
                return false;

            chunkParseOffset += 2;            
            return true;
        }

        // Проверка наличия данных чанка + \r\n
        if (buffer.size() < chunkParseOffset + chunkSize + 2)
            return false;

        body.insert(body.end(), buffer.begin() + chunkParseOffset, buffer.begin() + chunkParseOffset + chunkSize);
        chunkParseOffset += chunkSize;

        // Проверка \r\n после чанка
        if (!(buffer[chunkParseOffset] == '\r' && buffer[chunkParseOffset + 1] == '\n'))
            return false;
       
        chunkParseOffset += 2;
    }
}

void HttpRequestParser::printRequest() const {
    std::cout << "-------------------Request parsed --------------------------" << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "URL: " << url << std::endl;
    std::cout << "HostName: " << hostName << std::endl;
    std::cout << "KeepAlive: " << keepAlive << std::endl;
    std::cout << "HTTP Version: " << httpVersion << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body: ";
    for (size_t i = 0; i < body.size(); i++) {
        std::cout << body[i];
    }
    std::cout << std::endl;
    std::cout << "Query: " << query << std::endl;
    std::cout << "------------------End of Request parsed------------------------------\n\n";
}

const std::string& HttpRequestParser::getMethod() const {
    return method;
}

const std::string& HttpRequestParser::getUrl() const {
    return url;
}

const std::vector<char>& HttpRequestParser::getBody() const {
    return body;
}

const std::string& HttpRequestParser::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    static const std::string emptyStr = "";
    return emptyStr;
}

std::map<std::string, std::string>& HttpRequestParser::getHeaders() {
    return headers;
}

std::string &HttpRequestParser::getPath()
{
    return (path);
}
