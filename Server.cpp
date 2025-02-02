#include "Server.hpp"
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>

Server::Server(const std::string &config) 
{
	parseConfig(config);
	//socketManager.bindAndListen(8081); // Пример с портом 8081
}

Server::~Server() 
{
	// Закрытие всех сокетов
}

void Server::parseConfig(const std::string &config)
{
	if (!ConfigParser::parseConfig(config, servers)) 
	{
		throw std::runtime_error("Ошибка при парсинге конфигурационного файла");
	}

	if (servers.empty()) 
	{
		throw std::runtime_error("Не найдено ни одного сервера в конфигурационном файле");
	}

	for (size_t i = 0; i < servers.size(); ++i) 
	{
		std::cout << "-----------------Сервер " << i << ":----------------------------" << std::endl;
		servers[i].print();
	}
}

void Server::loop()
{
	// Создаем и настраиваем сокет с использованием SocketManager
	for (size_t i = 0; i < servers.size(); i++)
	{
		//TODO save sockets in vector
		//TODO nuzno tolko uniq serv+port
		//socketManager.bindSocket(std::atoi(servers[i].listen.c_str()));  
	}
	// Пример использования порта 8081
	socketManager.bindSocket(8081);
	socketManager.bindSocket(8083);

	std::cerr << "// ********************************************************";

	//fd_set master_set;  // Множество файловых дескрипторов
	fd_set read_fds;    // Множество файловых дескрипторов, готовых для чтения
	fd_set write_fds; 
	//int max_fd;         // Максимальный дескриптор для select

	// Инициализация master_set
	//FD_ZERO(&master_set);  // Обнуляем master_set
	//FD_SET(socketManager.getServerFd(), &master_set);  // Добавляем серверный сокет в master_set
	//max_fd = socketManager.getServerFd();  // Изначально максимальный дескриптор равен серверному

	while (true) 
	{
		//read_fds = master_set; // Копируем master_set в read_fds

		// Ожидаем активности на одном из сокетов
		if (!socketManager.isActive(read_fds, write_fds))
			continue;

		// Обрабатываем сокеты, готовые для чтения
		for (int i = 0; i <= socketManager.getMax_fd(); i++)
		{
			if (FD_ISSET(i, &read_fds))
			{
				if (socketManager.isSocket(i))
				{
					socketManager.acceptConnection(i);
					continue;
				}

				// Чтение HTTP-запроса от клиента
				char buffer[1024];
				ssize_t bytes_read = read(i, buffer, sizeof(buffer) - 1);
				if (bytes_read <= 0) 
				{
					// Если ошибка при чтении или клиент закрыл соединение
					if (bytes_read == 0) 
						std::cout << "Клиент отключился" << std::endl;
					else 
						std::cerr << "Ошибка при чтении данных!" << std::endl;
					socketManager.close(i);
					continue;
				}

				buffer[bytes_read] = '\0'; // Завершаем строку
				std::cout << "Получен запрос: \n" << buffer << std::endl;

				//TODO need to find a right website
				
				// Формирование HTTP-ответа
				const char* http_response =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n\r\n"
					"<html><body><h1>Hello, World!</h1></body></html>";

				// Отправка ответа клиенту
				write(i, http_response, strlen(http_response));
				socketManager.close(i);
			}
		}
	}
}
