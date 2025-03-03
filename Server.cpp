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
	char buffer[2049];
	ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

	if (bytes_read <= 0)
	{
		// Если ошибка при чтении или клиент закрыл соединение
		if (bytes_read == 0) 
			std::cout << "Клиент отключился" << std::endl;
		else
			std::cerr << "Ошибка при чтении данных!" << std::endl;
		close(fd);
		deletefds.push_back(fd);
		return;
	}

	buffer[bytes_read] = '\0'; // Завершаем строку
	std::cout << "Получен запрос: \n" << buffer << std::endl;

	// Request & Response
	HttpRequestParser request;
	request.parse(buffer);
	request.printRequest();
	//Response response(Response::FILE, 0, "", "", "/var/www/example");
	Response response = response.handleRequest(request.getMethod(), request.getUrl(), request.getBody().size());
    response.print();


	//TODO need to find a right website

	//TODO if (connection == close)
	//	close(fd);

	std::ifstream file("web1/index.html");
	if (!file)
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
	
	//bytes_read = read(f, buffer, sizeof(buffer) - 1);
	//bytes_read = recv(f, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
	file.read(buffer, sizeof(buffer));
	buffer[file.gcount()] = '\0';
	std::cerr << buffer;
	file.close();
	// Отправка ответа клиенту
	//if (false)
	send(fd, http_response, strlen(http_response), MSG_DONTWAIT | MSG_NOSIGNAL);
	send(fd, buffer, strlen(buffer), MSG_DONTWAIT | MSG_NOSIGNAL);
	close(fd);
	deletefds.push_back(fd);
}

void Server::execWrite(int fd, std::vector<int> &deletefds)
{
	close(fd);
	deletefds.push_back(fd);
}

void print(std::vector<struct pollfd> p)
{
	for (size_t i = 0; i < p.size(); i++)
	{
		std::cerr << p[i].fd << " - ";
		std::cerr << p[i].events << " - ";
		std::cerr << p[i].revents << ",   ";
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
	// Пример использования порта 8080
	socketManager.bindSocket(8080);

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
			//sockets fds
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
			//TODO read fds
			if (fds[i].revents & POLLIN)
			{
				execRead(fds[i].fd, deletefds);
			}

			//TODO write fds
			if (fds[i].revents & POLLOUT)
			{
				execWrite(fds[i].fd, deletefds);
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
