#include "HttpResponse.hpp"

Response::Response(Type type, int code, const std::string& message, const std::string& destination, const std::string& filePath)
    : type(type), code(code), message(message), destination(destination), filePath(filePath), urlLocal("") {}

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
    }
    std::cout << "----------------------------------------------------------" << std::endl;
}

// Функция для поиска Location по URL
int Response::getLocation(const ServerConfig& config, const std::string& url) {
    std::string url_to_test = url;
    // Поиск по полному пути или его частям
    std::cerr << "urlToTest " <<  url_to_test << "\n";
    while (url_to_test != "/") {
        // Ищем путь в locations
        for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
             it != config.locations.end(); ++it) {
            std::cerr << "location: " << it->first << "+++";
            if (it->first == url_to_test) {
                urlLocal = url.substr(it->first.length());
                if (urlLocal == "/")
                    urlLocal = "";
                std::cerr << "urlLocal " <<  urlLocal << "\n";
                // Возвращаем индекс (позицию) найденного Location
                return std::distance(config.locations.begin(), it);
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
            return std::distance(config.locations.begin(), it);
        }
    }
    return -1;
}

const ServerConfig::Location* getLocationByIndex(const ServerConfig& config, int index) {
    int i = 0;
    for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
         it != config.locations.end(); ++it, ++i) {
        if (i == index) {
            return &(it->second);
        }
    }
    return NULL;
}

bool isMethodAllowed(const ServerConfig& config, const std::string& method, int location_index) {
    const ServerConfig::Location& location = *getLocationByIndex(config, location_index);
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
    std::string str = config.client_max_body_size;
    if(str == "") {
        str = "1000000"; // какой макс????? в конфиге у нас два в сервере и локэйшене ??
    }
    char* end;
    unsigned long maxBodySize = std::strtoul(str.c_str(), &end, 10);
    std::cerr << "maxBodySize: " << maxBodySize << "+++";
    std::cerr << "size: " << size << "+++";
    return size <= maxBodySize;
}

std::string findRedirectPath(const ServerConfig& config, int location_index) {
    //разделить 301 и гугл
    const ServerConfig::Location& location = *getLocationByIndex(config, location_index);
    std::cerr << "location.path: " << location.path << " redirect: " << location.redirect << "+++";
    if (!location.redirect.empty()) {
        return location.redirect;
    }
    return "";
}

std::string Response::findLocalPath(const ServerConfig& config, const std::string& url, int location_index) {
    const ServerConfig::Location& location = *getLocationByIndex(config, location_index);
    std::string fullpath = location.root + urlLocal;
    std::cerr << "\nlocation.root: " << url << " +++ " <<  location.root << " +++ " << fullpath << "+++ \n" ;
    if (!fullpath.empty() && fullpath[0] == '/') {
        fullpath.erase(0, 1); // Удаляем первый символ
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

bool hasIndexFile(const std::string& folderPath) {
    // Список возможных индексных файлов
    const std::string indexFiles[] = {"index.html", "index.php", "index.htm"};
    const int numIndexFiles = sizeof(indexFiles) / sizeof(indexFiles[0]);
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        std::cerr << "Ошибка открытия директории: " << folderPath << std::endl;
        return false;
    }
    // Читаем содержимое директории
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string fileName = entry->d_name;
        for (int i = 0; i < numIndexFiles; ++i) {
            if (fileName == indexFiles[i]) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}

bool isAutoIndexEnabled(const ServerConfig& config, int location_index) {
    //  включен ли автоиндекс
    const ServerConfig::Location& location = *getLocationByIndex(config, location_index);
    return location.autoindex;
}

static Response generateFolderList(const std::string& folderPath) {
    std::vector<std::string> folders;
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        return Response(Response::ERROR, 404, "Directory not found");
    }
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue; // Пропускаем текущую и родительскую директории
        std::string fullPath = folderPath + "/" + name;
        struct stat statBuf;
        if (stat(fullPath.c_str(), &statBuf) == 0 && S_ISDIR(statBuf.st_mode)) {
            folders.push_back(name); // Добавляем только папки
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

bool isCGIExtension(const std::string& extension) {
    // является ли расширение CGI
    (void)extension;
    return false;
}

std::string Response::getPath(const ServerConfig& config, const std::string& url)
{
    int location_index = getLocation(config, url);
    const ServerConfig::Location& location = *getLocationByIndex(config, location_index);
    std::string fullpath = location.root + url;
    if (!fullpath.empty() && fullpath[0] == '/')
    {
        fullpath.erase(0, 1); // Удаляем первый символ
    }
    if (fullpath.empty() || url[url.length() - 1] == '/')
		fullpath = fullpath + "index.html";
    return fullpath;
}

Response Response::handleRequest(const ServerConfig& config, const std::string& method, const std::string& url, size_t bodySize) {
    int location_index = getLocation(config, url);
    std::cerr << "location index: " << location_index << "+++";
    if (location_index == -1) {
        std::cerr << "nooo getlocation" << std::endl;
        return Response(Response::ERROR, 404, "Not Found");
    }

    if (!isMethodAllowed(config, method, location_index)) {
        return Response(Response::ERROR, 405, "Method Not Allowed");
    }

    if (!isBodySizeValid(config, bodySize)) {
        return Response(Response::ERROR, 413, "Payload Too Large");
    }

    std::string redirectPath = findRedirectPath(config, location_index);
    if (!redirectPath.empty()) {
        int status_code;
        std::string url;
        std::istringstream iss(redirectPath);
        iss >> status_code;
        std::getline(iss, url);
        if (!url.empty() && url[0] == ' ')
            url.erase(0, 1);
        return Response(Response::REDIRECT, status_code, "", url, redirectPath);
    }

    std::string localPath = findLocalPath(config, url, location_index);
    std::cerr << "\nlocation index: " << url << " +++ " << location_index << " +++ " << localPath << " +++ \n" ;
    if (localPath.empty()) {
        return Response(Response::ERROR, 404, "Path Not Found");
    }

    if (method == "DELETE") {
        if (std::remove(localPath.c_str()) == 0) {
            return Response(Response::FILE, 204, "File deleted", "", localPath);
        } else {
            return Response(Response::ERROR, 500, "Failed to delete file");
        }
    }

    /* for what? we check it in findLocalPath
    if (!fileExists(localPath)) {
        return Response(Response::ERROR, 404, "File Not Found");
    }*/

    if (isFolder(localPath)) {
        std::cout << "Folder: " << localPath << std::endl;
        if (hasIndexFile(localPath)) {
            //return Response(Response::ERROR, 403, "Forbidden");
            if (isCGIExtension(localPath))
                return Response(Response::FILE, 200, "CGI Execution", "", localPath + "/index.html");
            else
                return Response(Response::FILE, 200, "", "", localPath + "/index.html");
        } else if (isAutoIndexEnabled(config, location_index)) {
            std::cout << "Autoindex enabled" << std::endl;
            return generateFolderList(localPath);
        } else
            return Response(Response::ERROR, 403, "Forbidden");
    }

    if (isCGIExtension(localPath))
        return Response(Response::FILE, 200, "CGI Execution", "", localPath);
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
        std::cerr << "TYPE!   filePath: " << filePath << std::endl;
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
                std::cerr << "file reading ... " << filePath << "!" << std::endl;
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
        default:
            response << "<html><body><h1>500 Internal Server Error</h1><p>Something went wrong.</p></body></html>";
            break;
    }

    std::cout << "\n-------------------------RESPONSE------------------------" << std::endl;
    std::cout << response.str().substr(0, 500) << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    return response.str();
}


