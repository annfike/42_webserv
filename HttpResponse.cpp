#include "HttpResponse.hpp"

void removeQuery(std::string& url) {
    // Находим индекс символа '?'
    size_t pos = url.find('?');

    // Если символ '?' найден, обрезаем строку до этого символа
    if (pos != std::string::npos) {
        url = url.substr(0, pos);  // Оставляем только часть до '?'
    }
}

Response::Response(Type type, int code, const std::string& message, const std::string& destination,
                   const std::string& filePath, const std::string& cgi_output)
    : type(type), code(code), message(message), destination(destination), filePath(filePath), cgi_output(cgi_output) {
        // std::cout << "Response constructor called with type: " << type << std::endl;
    }

void Response::print() const {
    std::cout << "-------------------Response-------------------------------: " << std::endl;
    switch (type) {
        case ERROR:
            std::cerr << "Error: " << code << " " << message << std::endl;
            break;
        case REDIRECT:
            std::cerr << "Redirect: " << destination << " " << code << std::endl;
            break;
        case FOLDER_LIST:
            std::cerr << "Folder list generated" << std::endl;
            std::cerr << "Folder: " << message << std::endl;
            break;
        case FILE:
            std::cerr << "File: " << filePath << std::endl;
            break;
        case CGI:
            std::cerr << "CGI: " << cgi_output << std::endl;
            break;
    }
    std::cout << "----------------------------------------------------------" << std::endl;
}

// Функция для поиска Location по URL
const ServerConfig::Location getLocation(const ServerConfig& config, const std::string& url, std::string* urlLocal) {
    std::string url_to_test = url;
    // Поиск по полному пути или его частям
    while (!url_to_test.empty() && url_to_test != "/") {
        // Ищем путь в locations
        for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
            it != config.locations.end(); ++it) {
            std::cerr << "location: " << it->first << "+++";
            if (it->first == url_to_test) {
                *urlLocal = url.substr(it->first.length());
                // Возвращаем индекс (позицию) найденного Location
                return it->second;
            }
        }
        // Укорачиваем путь
        size_t last_slash = url_to_test.find_last_of('/');
        if (last_slash == std::string::npos) {
            break;
        }
        url_to_test = url_to_test.substr(0, last_slash);
        if (url_to_test.empty()) {
            url_to_test = "/";
        }
    }
    // Проверка корневого пути
    for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
        it != config.locations.end(); ++it) {
        if (it->first == "/") {
            *urlLocal = url;
            return it->second;
        }
    }
    return ServerConfig::Location();
}

bool isMethodAllowed(const ServerConfig::Location& location, const std::string& method) {
    // Проверяем, есть ли метод в списке разрешенных методов
    for (std::vector<std::string>::const_iterator methodIt = location.methods.begin();
        methodIt != location.methods.end(); ++methodIt) {
        std::cerr << "method: " << method << "---";
        std::cerr << *methodIt << "+++";
        if (*methodIt == method) {
            return true; // Метод разрешен
        }
    }
    return false;
}

bool isBodySizeValid(const ServerConfig& config, size_t size) {
    return size <= config.client_max_body_size;
}

std::string findRedirectPath(const ServerConfig::Location& location) {
    //разделить 301 и гугл
    std::cerr << "location.path: " << location.path << " redirect: " << location.redirect << "+++";
    if (!location.redirect.empty()) {
        return location.redirect;
    }
    return "";
}

std::string findLocalPath(const ServerConfig::Location& location, std::string& urlLocal) {
    std::string fullpath = location.root + urlLocal;
    std::cerr << "/*" << fullpath << "|" << location.root << "|" << urlLocal << "|" << "*/";
    if (!fullpath.empty() && fullpath[0] == '/') {
        fullpath = fullpath.substr(1); // Удаляем первый символ
    }
    // Проверка существования пути
    if (access(fullpath.c_str(), F_OK) != -1) {
        std::cout << "Path exists: " << fullpath << std::endl;
        return fullpath; // Путь существует, возвращаем его
    }
    return "";
}

bool fileExists(const std::string& path) {
    // проверка существования файла // for what? we check it in findLocalPath
    (void)path;
    return false;
}

bool isFolder(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        std::cerr << "Ошибка доступа к пути: " <<  std::endl;
        return false;
    }
    // Проверка, является ли путь директорией
    return (info.st_mode & S_IFDIR) != 0;
}

std::string getIndexFile(const ServerConfig::Location& location, const std::string& folderPath) {
    // Список возможных индексных файлов
    const std::string indexFiles[] = {location.index, "index.html", "index.php", "index.htm"};
    const int numIndexFiles = sizeof(indexFiles) / sizeof(indexFiles[0]);
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        std::cerr << "Ошибка открытия директории: " << folderPath << std::endl;
        return "";
    }
    // Читаем содержимое директории
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string fileName = entry->d_name;
        for (int i = 0; i < numIndexFiles; ++i) {
            if (fileName == indexFiles[i]) {
                closedir(dir);
                return fileName;
            }
        }
    }
    closedir(dir);
    return "";
}

Response getErrorResponse(const ServerConfig& config, int code, std::string message)
{
    std::map<int, std::string>::const_iterator err = config.error_pages.find(code);
    if (err != config.error_pages.end())
        return Response(Response::REDIRECT, 301, "", err->second);
    return Response(Response::ERROR, code, message);
}

Response generateFolderList(const ServerConfig& config, const std::string& folderPath) {
    std::vector<std::string> folders;
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        return getErrorResponse(config, 404, "Directory not found");
    }
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue; // Пропускаем текущую и родительскую директории
        std::string fullPath = folderPath + "/" + name;
        struct stat statBuf;
        //if (stat(fullPath.c_str(), &statBuf) == 0 && S_ISDIR(statBuf.st_mode)) { // Добавляем только папки
        if (stat(fullPath.c_str(), &statBuf) == 0) { // we add files also
            folders.push_back(name); 
        }
    }
    closedir(dir);
    // Формируем HTML-страницу с списком папок
    std::stringstream html;
    html << "<html><body><h1>Directory listing for " << folderPath << "</h1><ul>";
    for (size_t i = 0; i < folders.size(); ++i) {
        html << "<li><a href='" << folders[i] << "'>" << folders[i] << "</a></li>";
    }
    html << "</ul></body></html>";

    return Response(Response::FOLDER_LIST, 200, html.str());
}

void parseMultipartFormData(std::istream &request, const std::string &boundary, std::string& fileName, std::vector<char>& fileData, size_t length) 
{
    std::string line;
    std::string currentPart;
    std::string contentType;
    size_t bytesRead = 0;
    length = length - boundary.length() - 2 - 4; // '--' in the boundary, '--\r\n' in the end
    
    std::getline(request, line);
    bytesRead += line.size() + 1;
    if (line.find(boundary) == std::string::npos) 
        return;

    std::getline(request, line);
    bytesRead += line.size() + 1;
    if (line.find("Content-Disposition:") != std::string::npos) 
    {
        size_t filenamePos = line.find("filename=\"");
        if (filenamePos != std::string::npos) 
        {
            fileName = line.substr(filenamePos + 10);
            fileName = fileName.substr(0, fileName.find('"')); // Удаляем лишнее
            std::cerr << "filename=" << fileName;
        }
    }

    std::getline(request, line);
    bytesRead += line.size() + 1;
    if (line.find("Content-Type:") != std::string::npos)
    {}

    std::getline(request, line);
    bytesRead += line.size() + 1;
    if (line == "\r")
    {
        // Пропускаем пустую строку перед данными
    }

    std::cerr << "\nbytesRead:" << bytesRead <<", length:" << length << "\n";

    char buffer[4096];
    while (!request.eof() && bytesRead < length) {
        request.read(buffer, sizeof(buffer));
        size_t count = request.gcount();
        count = std::min(count, length - bytesRead);
        bytesRead += count;

        if (count == 0) 
            break;

        fileData.insert(fileData.end(), buffer, buffer + count);
    }
}

Response Response::handleRequest(const ServerConfig& config, HttpRequestParser request) {
    std::map<std::string, std::string> headers = request.getHeaders();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
        printf("  %s: %s\n", it->first.c_str(), it->second.c_str());
    }

    std::string urlLocal;
    std::string url = request.getUrl();
    ServerConfig::Location location = getLocation(config, url, &urlLocal);
    //std::cerr << "location index: " << location_index << "+++";
    if (location.path.empty() ) {
        std::cerr << "nooo getlocation" << std::endl;
        return getErrorResponse(config, 404, "Not Found");
    }

    if (!isMethodAllowed(location, request.getMethod())) 
        return getErrorResponse(config, 405, "Method Not Allowed");

    if (!isBodySizeValid(config, request.getBody().size())) 
        return getErrorResponse(config, 413, "Payload Too Large");

    std::string redirectPath = findRedirectPath(location);
    if (!redirectPath.empty()) {
        int status_code;
        std::string url;
        std::istringstream iss(redirectPath);
        iss >> status_code;
        std::getline(iss, url);
        if (!url.empty() && url[0] == ' ')
            url = url.substr(1);
        return Response(Response::REDIRECT, status_code, "", url, redirectPath);
    }

    removeQuery(urlLocal);
	std::string localPath = findLocalPath(location, urlLocal);
    // std::cerr << "\nlocation index1: " << url << " +++ " << " +++ " << localPath << " +++ \n" ;
    if (localPath.empty())
        return getErrorResponse(config, 404, "Path Not Found1");

    if (request.getMethod() == "DELETE") 
    {
        if (std::remove(localPath.c_str()) == 0) 
            return Response(Response::FILE, 204, "File deleted", "", localPath);
        else 
            return getErrorResponse(config, 500, "Failed to delete file");
    }

    if (request.getMethod() == "POST")
    {
        std::cout << "request.boundary: " << request.boundary << std::endl;
        if (request.boundary.empty())
            return getErrorResponse(config, 500, "Not supported!!!");

        std::string folder = location.upload_store;
        if (folder.empty())
            folder = location.root;

        std::stringstream bodys;
        bodys.write(request.getBody().data(), request.getBody().size());

        std::string filename;
        std::vector<char> fileData;
        parseMultipartFormData(bodys, request.boundary, filename, fileData, request.contentLength);
        filename = folder.substr(1) + '/' + filename;
        
        std::cout << "Saving to file: " << filename << "\n";

        // Сохраняем тело запроса в файл
        std::ofstream outFile(filename.c_str(), std::ios::binary);
        if (outFile && outFile.is_open()) {
            outFile.write(fileData.data(), fileData.size());
            outFile.close();

            return Response(Response::FILE, 200, "File uploaded successfully!");
        }
        else 
            return getErrorResponse(config, 500, "Error saving file!");
    }

    /* for what? we check it in findLocalPath
    if (!fileExists(localPath)) {
        return Response(Response::ERROR, 404, "File Not Found");
    }*/

    if (isFolder(localPath)) {
        std::cout << "Folder: " << localPath << std::endl;
        std::string iFile;
        if (localPath.empty() || url[url.length() - 1] != '/')
            return Response(Response::REDIRECT, 301, "", url + "/", "");
        else if (!(iFile = getIndexFile(location, localPath)).empty()) {
            if (CgiHandler().isCGIExtension(localPath))
                return Response(Response::FILE, 200, "CGI Execution", "", localPath + "/index.html");
            else
                return Response(Response::REDIRECT, 301, "", url + iFile);
        } else if (location.autoindex) {
            std::cout << "Autoindex enabled" << std::endl;
            return generateFolderList(config, localPath);
        } else
            return getErrorResponse(config, 403, "Forbidden");
    }

    if (CgiHandler().isCGIExtension(localPath)) {
        // Logger::logInfo("isCGIExtension() is running...");
        location.cgiPath = localPath;
        return CgiHandler().exec(location, request);
    }
    return Response(Response::FILE, 200, "", "", localPath);
}

// Метод для формирования HTTP-ответа
const std::string Response::toHttpResponse() const {
    std::ostringstream response;

    if (type == FILE && code == 204) { // DELETE запрос успешно выполнен
        std::cerr << "DELETE!   filePath: " << filePath << std::endl;
        response << "HTTP/1.1 204 No Content\r\n";
        response << "Content-Length: 0\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";

        std::string responseStr = response.str();
        char* httpResponse = new char[responseStr.size() + 1];
        std::strcpy(httpResponse, responseStr.c_str());
        return httpResponse;

        //проверка curl -X DELETE http://localhost:8001/bbb/1.txt
    }

    // Определяем Content-Type по расширению файла
    std::string contentType = "text/html"; // По умолчанию HTML
    if (type == FILE) {
        std::cout << "TYPE!   filePath: " << filePath << std::endl;
        std::string ext = filePath.substr(filePath.find_last_of('.') + 1);
        if (ext == "html" || ext == "htm") contentType = "text/html";
        else if (ext == "txt") contentType = "text/plain";
        else if (ext == "jpg" || ext == "jpeg") contentType = "image/jpeg";
        else if (ext == "png") contentType = "image/png";
        else if (ext == "gif") contentType = "image/gif";
        else if (ext == "css") contentType = "text/css";
        else if (ext == "js") contentType = "application/javascript";
        else if (ext == "json") contentType = "application/json";
        else if (ext == "pdf") contentType = "application/pdf";
        else if (ext == "ico") contentType = "image/x-icon";
    }

    // Добавляем статусную строку
    switch (type) {
        case ERROR:
            response << "HTTP/1.1 " << code << " Error\r\n";
            break;
        case REDIRECT:
            response << "HTTP/1.1 " << code << " Moved Temporarily\r\n";
            break;
        case FOLDER_LIST:
            response << "HTTP/1.1 " << code << " OK\r\n";
            break;
        case FILE:
            response << "HTTP/1.1 " << code << " OK\r\n";
            break;
        case CGI:  // Добавляем обработку для типа CGI
            response << "HTTP/1.1 " << code << " OK\r\n";
            break;
        default:
            response << "HTTP/1.1 500 Internal Server Error\r\n";
            break;
    }

    // Добавляем заголовки     
    //response << "Content-Type: text/html\r\n"; // По умолчанию тип содержимого — HTML  
    response << "Content-Type: " << contentType << "\r\n";  
    if (type == REDIRECT) {
        response << "Location: " << destination << "\r\n"; // Заголовок для редиректа
        response << "Content-Length: 0\r\n";
    }
    if (type == FILE)
    {
        std::ifstream file(filePath.c_str(), std::ifstream::binary | std::ifstream::ate);
        if (file)
        {
            std::streamsize fileSize = file.tellg(); 
            response << "Content-Length: " << fileSize << "\r\n";
            file.close();
        }
    }
    response << "Connection: close\r\n"; // Закрываем соединение после ответа
    response << "\r\n"; // Пустая строка между заголовками и телом

    // Добавляем тело ответа
    // std::cout << "type: " << type << std::endl;
    switch (type) {
        case ERROR:
            response << "<html><body><h1>Error " << code << "</h1><p>" << message << "</p></body></html>";
            break;
        case REDIRECT:
            response << "<html><body><h1>Redirecting...</h1><p>You are being redirected to <a href=\"" << destination << "\">" << destination << "</a>.</p></body></html>";
            break;
        case FOLDER_LIST:
            response << message; // message уже содержит HTML-список папок
            break;
        case FILE:
            {
                if (filePath.empty())
                    break;

                std::cout << "file reading ... " << filePath << "!" << std::endl;
                std::ifstream file(filePath.c_str());
                if (!file)
                    std::cerr << "Error reading file " << filePath << std::endl;
                else
                {
                    char buffer[4096];  // Буфер для чтения данных частями
                    while (file.read(buffer, 4096))
                    {
                        response.write(buffer, file.gcount());
                    }
                    if (file.gcount() > 0) { // If there are remaining bytes
                        response.write(buffer, file.gcount());
                    }
                    file.close();
                }
            }
            break;
        case CGI:
            {
                // Logger::logInfo("Inside block CGI!");
                std::string cgi_output_copy = cgi_output;
                const std::string contentTypeHeader = "Content-Type: text/html";
                size_t pos = cgi_output_copy.find(contentTypeHeader);

                if (pos != std::string::npos) {
                    size_t endPos = cgi_output_copy.find("\r\n", pos);
                    if (endPos != std::string::npos) {
                        cgi_output_copy.erase(pos, endPos - pos + 2);
                    }
                }

                response << cgi_output_copy;
                break;
            }
        default:
            response << "<html><body><h1>500 Internal Server Error</h1><p>Something went wrong.</p></body></html>";
            break;
    }

    std::cout << "\n-------------------------RESPONSE------------------------" << std::endl;
    std::cout << response.str()<< std::endl;;
    //std::cout << response.str().substr(0, 500) << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    return response.str();
}
