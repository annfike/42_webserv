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

// Функция для поиска Location по URL
int getLocation(const ServerConfig& config, const std::string& url) {
    std::string url_to_test = url;
    // Поиск по полному пути или его частям
    while (url_to_test != "/") {
        // Ищем путь в locations
        for (std::map<std::string, ServerConfig::Location>::const_iterator it = config.locations.begin();
             it != config.locations.end(); ++it) {
            std::cerr << "location: " << it->first << "+++";
            if (it->first == url_to_test) {
                // Возвращаем индекс (позицию) найденного Location
                return std::distance(config.locations.begin(), it);
            }
        }
        // Укорачиваем путь
        size_t last_slash = url.find_last_of('/');
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

std::string findLocalPath(const ServerConfig& config, const std::string& url, int location_index) {
    const ServerConfig::Location& location = *getLocationByIndex(config, location_index);
    std::string fullpath = location.root + url;
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

bool isCGIExtension(const std::string& extension) {
    // является ли расширение CGI
    (void)extension;
    return false;
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
        return Response(Response::REDIRECT, 301, "", redirectPath);
    }

    std::string localPath = findLocalPath(config, url, location_index);
    if (localPath.empty()) {
        return Response(Response::ERROR, 404, "Path Not Found");
    }

    /* for what? we check it in findLocalPath
    if (!fileExists(localPath)) {
        return Response(Response::ERROR, 404, "File Not Found");
    }*/

    if (isFolder(localPath)) {
        std::cout << "Folder: " << localPath << std::endl;
        if (hasIndexFile(localPath)) {
            //return Response(Response::ERROR, 403, "Forbidden");
            if (isCGIExtension(localPath)) {
                // Обработка CGI
            } else {
                return Response(Response::FILE, 0, "", "", localPath + "/index.html");
            }
        } else if (isAutoIndexEnabled(config, location_index)) {
            std::cout << "Autoindex enabled" << std::endl;  
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

