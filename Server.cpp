#include "Server.hpp"

Server::Server(const std::string &config) 
{
	parseConfig(config);
}

Server::~Server() 
{
	socketManager.closeSockets();
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

void Server::execRead(int fd, std::vector<int>& deletefds)
{
	std::cerr << fd << "/*/*/*/*/*/*/*/*\n";
	// Чтение HTTP-запроса от клиента
	char buffer[1024];
	ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
	if (bytes_read <= 0) 
	{
		// Если ошибка при чтении или клиент закрыл соединение
		if (bytes_read == 0) 
			std::cout << "Клиент отключился" << std::endl;
		else 
			std::cerr << "Ошибка при чтении данных!" << std::endl;
		socketManager.closeFd(fd);
		deletefds.push_back(fd);
		return;
	}

	buffer[bytes_read] = '\0'; // Завершаем строку
	std::cout << "Получен запрос: \n" << buffer << std::endl;

	//TODO need to find a right website
	
	int f = open("web1/index.html", O_RDONLY);
	if (!f)
		std::cerr << "Ошибка при чтении file!" << std::endl;

	// Формирование HTTP-ответа
	/*char* http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Connection: close\r\n\r\n"
		"<html><body><h1>Hello, World!</h1></body></html>";*/
	const char * http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Connection: close\r\n\r\n";
	bytes_read = read(f, buffer, sizeof(buffer) - 1);
	buffer[bytes_read] = '\0';
	std::cerr << buffer;
	close(f);
	// Отправка ответа клиенту
	//if (false)
	write(fd, http_response, strlen(http_response));
	write(fd, buffer, strlen(buffer));
	socketManager.closeFd(fd);
	deletefds.push_back(fd);
}

void print(std::vector<struct pollfd> p)
{
	for (size_t i = 0; i < p.size(); i++)
	{
		std::cerr << p[i].fd << " - ";
		std::cerr << p[i].events << " - ";
		std::cerr << p[i].revents << ", ";
	}
	std::cerr << std::endl;
}

void Server::loop()
{
	// Создаем и настраиваем сокет с использованием SocketManager
	for (size_t i = 0; i < servers.size(); i++)
	{
		//TODO nuzno tolko uniq serv+port
		socketManager.bindSocket(std::atoi(servers[i].listen.c_str()));
	}
	// Пример использования порта 8081
	socketManager.bindSocket(8081);
	socketManager.bindSocket(8083);

	std::cerr << "// ********************************************************";

	std::vector<struct pollfd> fds(socketManager.sockets);    // Множество файловых дескрипторов, готовых для чтения
	std::vector<struct pollfd> newfds;
	std::vector<int> deletefds;
	while (true)
	{
		// Ожидаем активности на одном из сокетов
		print(fds);
		if (!socketManager.getActive(fds))
			continue;

		for (int i = 0; i < (int)fds.size(); i++)
		{
			std::cerr << fds.size() << " razmer\n";
			if (socketManager.isSocket(fds[i].fd))
			{
				if (fds[i].revents & (POLLIN | POLLOUT))
				{
					std::cerr << fds[i].fd << " - socket\n";
					struct pollfd fd = socketManager.acceptConnection(fds[i]);
					if (fd.fd != -1)
						newfds.push_back(fd);
				}
				continue;
			}
			print(fds);
			if (fds[i].revents & POLLIN)
			{
				execRead(fds[i].fd, deletefds);
			}

			if (fds[i].revents & POLLOUT)
			{
				//execRead(fds[i].fd);
				close(fds[i].fd);
				deletefds.push_back(fds[i].fd);
			}
		}
		fds.insert(fds.end(), newfds.begin(), newfds.end());
		newfds.clear();

		for (size_t i = 0; i < deletefds.size(); ++i) 
		{
			for (size_t j = 0; j < fds.size(); ++j) 
			{
				if (fds[j].fd == deletefds[i]) 
				{
					fds.erase(fds.begin() + j);
					break;
				}
			}
		}
		deletefds.clear();
	}
}
