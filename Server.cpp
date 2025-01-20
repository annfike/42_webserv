#include "Server.hpp"
#include <stdexcept>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <cstring>


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

#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

void Server::loop() {
    // Создаем и настраиваем сокет с использованием SocketManager
    socketManager.bindAndListen(8081);  // Пример использования порта 8081

    fd_set master_set;  // Множество файловых дескрипторов
    fd_set read_fds;    // Множество файловых дескрипторов, готовых для чтения
    int max_fd;         // Максимальный дескриптор для select

    // Инициализация master_set
    FD_ZERO(&master_set);  // Обнуляем master_set
    FD_SET(socketManager.getServerFd(), &master_set);  // Добавляем серверный сокет в master_set
    max_fd = socketManager.getServerFd();  // Изначально максимальный дескриптор равен серверному

    while (true) {
        read_fds = master_set;  // Копируем master_set в read_fds

        // Ожидаем активности на одном из сокетов
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "Ошибка при вызове select!" << std::endl;
            continue;
        }

        // Проверяем, если серверный сокет готов для принятия соединений
        if (FD_ISSET(socketManager.getServerFd(), &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            // Принятие нового соединения
            int client_fd = accept(socketManager.getServerFd(), (struct sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) {
                std::cerr << "Ошибка при принятии соединения!" << std::endl;
                continue;
            }

            // Добавляем клиентский сокет в master_set
            FD_SET(client_fd, &master_set);
            if (client_fd > max_fd) {
                max_fd = client_fd;  // Обновляем максимальный дескриптор
            }

            std::cout << "Новое соединение принято" << std::endl;
        }

        // Обрабатываем сокеты, готовые для чтения
        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == socketManager.getServerFd()) {
                    // Это серверный сокет, уже обработан выше
                    continue;
                }

                // Чтение HTTP-запроса от клиента
                char buffer[1024];
                ssize_t bytes_read = read(i, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    // Если ошибка при чтении или клиент закрыл соединение
                    if (bytes_read == 0) {
                        std::cout << "Клиент отключился" << std::endl;
                    } else {
                        std::cerr << "Ошибка при чтении данных!" << std::endl;
                    }
                    close(i);  // Закрытие соединения с клиентом
                    FD_CLR(i, &master_set);  // Удаляем клиентский сокет из master_set
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
                write(i, http_response, strlen(http_response));

                // Закрытие соединения с клиентом
                close(i);
                FD_CLR(i, &master_set);  // Удаляем клиентский сокет из master_set
            }
        }
    }
}
