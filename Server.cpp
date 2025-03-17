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
		throw std::runtime_error("Error parsing configuration file!");
	}

	if (servers.empty())
	{
		throw std::runtime_error("No servers found in configuration file!");
	}

	for (size_t i = 0; i < servers.size(); ++i)
	{
		std::cout << "-----------------Server " << i << ":----------------------------" << std::endl;
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
		if (bytes_read < 0) 
			std::cerr << "Error reading request body: " << strerror(errno) << std::endl;
		socketManager.closeConnection(con);
		return;
	}

	buffer[bytes_read] = '\0'; // Завершаем строку
	std::cout << "Request received: \n" << buffer << std::endl;

	// Request & Response
	HttpRequestParser request;
	request.parse(buffer);
	request.printRequest();
	//Response response(Response::FILE, 0, "", "", "/var/www/example");
	std::cerr << request.hostName << std::endl;
	ServerConfig& config = con.getConfig(request.hostName);
	Response response = response.handleRequest(config, request.getMethod(), request.getUrl(), request.getBody().size());
	response.print();

	//TODO if (connection == close)
	//	close(fd);

	//con.config.locations;

	if (request.getUrl() == "/favicon.ico")
	{
		socketManager.closeConnection(con);
		return;
	}

	const char * http_response = response.toHttpResponse();
	std::cout  << std::endl << "----------------------------------------------------------" << std::endl;
	
	// Отправка ответа клиенту
	//if (false)
	send(con.poll.fd, http_response, strlen(http_response), MSG_DONTWAIT | MSG_NOSIGNAL);
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
