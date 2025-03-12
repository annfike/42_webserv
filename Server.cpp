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

void Server::execRead(Connection con)
{
	std::cout << "----------------- READING FROM FD=" << con.poll.fd << " -----------------" <<std::endl;
	
	// Чтение HTTP-запроса от клиента
	char buffer[2049];
	ssize_t bytes_read = read(con.poll.fd, buffer, sizeof(buffer) - 1);

	if (bytes_read <= 0)
	{
		// Если ошибка при чтении или клиент закрыл соединение
		if (bytes_read == 0) 
			std::cout << "Клиент отключился" << std::endl;
		else
			std::cerr << "Ошибка при чтении данных!" << std::endl;
		socketManager.closeConnection(con);	
		return;
	}

	buffer[bytes_read] = '\0'; // Завершаем строку
	std::cout << "Получен запрос: \n" << buffer << std::endl;

	// Request & Response
	HttpRequestParser request;
	request.parse(buffer);
	request.printRequest();
	//Response response(Response::FILE, 0, "", "", "/var/www/example");
	Response response = response.handleRequest(con.config, request.getMethod(), request.getUrl(), request.getBody().size());
	response.print();


	//TODO need to find a right website

	//TODO if (connection == close)
	//	close(fd);

	//con.config.locations;

	if (request.getUrl() == "/favicon.ico")
	{
		socketManager.closeConnection(con);
		return;
	}

	std::string path = response.getPath(con.config, request.getUrl());
	std::cerr << std::endl;
	std::cerr << "Path=" << path << std::endl;
	
	std::ifstream file(path.c_str());
	if (!file)
		std::cerr << "Ошибка при чтении file!" << std::endl;

	// Формирование HTTP-ответа
	/*char* http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Connection: close\r\n\r\n"
		"<html><body><h1>Hello, World!</h1></body></html>";*/
	/*const char * http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Connection: close\r\n\r\n";*/
	const char * http_response = response.toHttpResponse();
	std::cout << http_response;
	std::cout  << std::endl << "----------------------------------------------------------" << std::endl;
	
	//bytes_read = read(f, buffer, sizeof(buffer) - 1);
	//bytes_read = recv(f, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
	file.read(buffer, sizeof(buffer));
	buffer[file.gcount()] = '\0';
	std::cerr << buffer;
	file.close();
	// Отправка ответа клиенту
	//if (false)
	send(con.poll.fd, http_response, strlen(http_response), MSG_DONTWAIT | MSG_NOSIGNAL);
	send(con.poll.fd, buffer, strlen(buffer), MSG_DONTWAIT | MSG_NOSIGNAL);
	socketManager.closeConnection(con);
}

void Server::execWrite(Connection con)
{
	socketManager.closeConnection(con);
}

void print(std::vector<Connection> cons)
{
	std::cout << std::endl;
	std::cout << "Fds statuses:" << std::endl;
	for (size_t i = 0; i < cons.size(); i++)
	{
		std::cout << "	FD=" << cons[i].poll.fd;
		std::cout << " E=" << cons[i].poll.events;
		std::cout << " R=" << cons[i].poll.revents << std::endl;
	}
	std::cout << std::endl;
}

void Server::loop()
{
	std::cout << std::endl;
	std::cout << "Creating sockets:" << std::endl;
	// Создаем и настраиваем сокет с использованием SocketManager
	for (size_t i = 0; i < servers.size(); i++)
	{
		//TODO nuzno tolko uniq serv+port
		socketManager.bindSocket(servers[i]);
	}
	// Пример использования порта 8080
	//socketManager.bindSocket("0.0.0.0", 8080);
	std::cout << std::endl;

	while (true)
	{
		// Ожидаем активности на одном из сокетов
		std::vector<Connection> cons = socketManager.getActiveConnections();
		if (cons.empty())
			continue;

		print(cons);
		for (size_t i = 0; i < cons.size(); i++)
		{
			if (cons[i].isSocket)
			{
				socketManager.acceptConnection(cons[i]);
				continue;
			}

			if (cons[i].poll.revents & POLLIN)
				execRead(cons[i]);
			if (cons[i].poll.revents & POLLOUT)
				execWrite(cons[i]);
		}
	}
}
