#include "HttpResponse.hpp"

Response::Response(Type type, int code, const std::string& message, const std::string& destination, const std::string& filePath)
    : type(type), code(code), message(message), destination(destination), filePath(filePath) {}

void Response::print() const {
    std::cout << "-------------------Response-------------------------------: ";
    switch (type) {
        case ERROR:
            std::cerr << "Error: " << code << " " << message << std::endl;
            break;
        case REDIRECT:
            std::cerr << "Redirect: " << destination << " " << code << std::endl;
            break;
        case FOLDER_LIST:
            std::cerr << "Folder list generated" << std::endl;
            break;
        case FILE:
            std::cerr << "File: " << filePath << std::endl;
            break;
    }
    std::cout << "----------------------------------------------------------" << std::endl;
}


/*bool isMethodAllowed(const std::string& method) {
    // проверка допустимости метода
    (void)method;
    return false;
}*/

//Как добраться до конфига? у нас фд в execRead
//Функция для проверки допустимости метода 
bool isMethodAllowed(const ServerConfig& config, const std::string& method, const std::string& url) {
    // Ищем Location, соответствующий URL
    for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
         it != config.locations.end(); ++it) {
            std::cerr << "location ==========================================";
        const std::string& locationPath = it->first;
        const ServerConfig::Location& location = it->second;
        // Проверяем, соответствует ли URL текущему Location
        if (url.find(locationPath) != std::string::npos) {

            // Проверяем, есть ли метод в списке разрешенных методов
            for (std::vector<std::string>::const_iterator methodIt = location.methods.begin();
                 methodIt != location.methods.end(); ++methodIt) {
                    std::cerr << *methodIt << "+++";
                if (*methodIt == method) {
                    return true; // Метод разрешен
                }
            }            
            return false; // Метод не разрешен
        }
    }
    // Если Location для URL не найден, возвращаем false
    return false;
}

bool isBodySizeValid(size_t size) {
    // проверка размера тела запроса
    (void)size;
    return false;
}

std::string findRedirectPath(const std::string& url) {
    // поиск пути перенаправления
    (void)url;
    return "";
}

std::string findLocalPath(const std::string& url) {
    // поиск локального пути
    (void)url;
    return "";
}

bool fileExists(const std::string& path) {
    // проверка существования файла
    (void)path;
    return false;
}

bool isFolder(const std::string& path) {
    // является ли путь папкой
    (void)path;
    return false;
}

bool hasIndexFile(const std::string& folderPath) {
    // проверка наличия индексного файла в папке
    (void)folderPath;
    return false;
}

bool isAutoIndexEnabled() {
    //  включен ли автоиндекс
    return false;
}

bool isCGIExtension(const std::string& extension) {
    // является ли расширение CGI
    (void)extension;
    return false;
}

Response Response::handleRequest(const ServerConfig& config, const std::string& method, const std::string& url, size_t bodySize) {
    if (!isMethodAllowed(config, method, url)) {
        return Response(Response::ERROR, 405, "Method Not Allowed");
    }

    if (!isBodySizeValid(bodySize)) {
        return Response(Response::ERROR, 413, "Payload Too Large");
    }

    std::string redirectPath = findRedirectPath(url);
    if (!redirectPath.empty()) {
        return Response(Response::REDIRECT, 301, "", redirectPath);
    }

    std::string localPath = findLocalPath(url);
    if (localPath.empty()) {
        return Response(Response::ERROR, 404, "File Not Found");
    }

    if (!fileExists(localPath)) {
        return Response(Response::ERROR, 404, "File Not Found");
    }

    if (isFolder(localPath)) {
        if (hasIndexFile(localPath)) {
            return Response(Response::ERROR, 403, "Forbidden");
        } else if (isAutoIndexEnabled()) {
            return Response(Response::FOLDER_LIST);
        } else {
            return Response(Response::ERROR, 403, "Forbidden");
        }
    }

    if (isCGIExtension(localPath)) {
        // Обработка CGI
    }

    return Response(Response::FILE, 0, "", "", localPath);
}

