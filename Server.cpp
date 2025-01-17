#include "Server.hpp"
#include <stdexcept>
#include <string.h>

Server::Server(const std::string &config) {
    parseConfig(config);
    //socketManager.bindAndListen(8081); // Пример с портом 8081
}

Server::~Server() {
    // Закрытие всех сокетов
}

void Server::parseConfig(const std::string &config) {
    if (!ConfigParser::parseConfig(config, servers)) {
        throw std::runtime_error("Ошибка при парсинге конфигурационного файла");
    }

    if (servers.empty()) {
        throw std::runtime_error("Не найдено ни одного сервера в конфигурационном файле");
    }

    for (size_t i = 0; i < servers.size(); ++i) {
        servers[i].print();
    }
}

void Server::loop() {
    // Создаем и настраиваем сокет с использованием SocketManager
    socketManager.bindAndListen(8081);  // Пример использования порта 8081

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Используем server_fd из SocketManager
        int client_fd = accept(socketManager.getServerFd(), (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Ошибка при принятии соединения!" << std::endl;
            continue;
        }

        // Обработка клиентского соединения
        std::cout << "Новое соединение принято" << std::endl;
        //close(client_fd);

        // Чтение HTTP-запроса от клиента
        char buffer[1024];
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            std::cerr << "Ошибка при чтении данных!" << std::endl;
            close(client_fd);
            continue;
        }
        buffer[bytes_read] = '\0'; // Завершаем строку
        std::cout << "Получен запрос: \n" << buffer << std::endl;

        // Формирование HTTP-ответа
        const char* http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>Hello, World!</h1></body></html>";

        // Отправка ответа клиенту
        write(client_fd, http_response, strlen(http_response));

        // Закрытие соединения с клиентом
        close(client_fd);
    }
}

