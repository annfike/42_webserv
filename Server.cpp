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

	ServerConfig::Location errLoc;
	errLoc.methods.push_back("GET");
	errLoc.root = "/pages/error_pages";
	errLoc.path = "/error_pages";
	errLoc.autoindex = false;
	for (size_t i = 0; i < servers.size(); ++i)
	{
		servers[i].locations[errLoc.root] = errLoc;
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
	std::vector<char> accumulatedData;
	char buffer[4096];
	ssize_t bytes_read;

	while ((bytes_read = read(con.poll.fd, buffer, sizeof(buffer) - 1)) > 0)
    {
		accumulatedData.insert(accumulatedData.end(), buffer, buffer + bytes_read); // Добавляем в накопленные данные
	}
	if (bytes_read <= 0 && accumulatedData.empty())
	{
		// Если ошибка при чтении или клиент закрыл соединение
		if (bytes_read < 0) 
			std::cerr << "Error reading request body: " << strerror(errno) << std::endl;
		socketManager.closeConnection(con);
		return;
	}

	std::cout << "Request received: \n";
    for (size_t i = 0; i < std::min(accumulatedData.size(), size_t(500)); i++) {
        std::cout << accumulatedData[i];
    }
    std::cout << std::endl;

	// Request & Response
	HttpRequestParser request;
	try
    {
        request.parse(accumulatedData);  // Парсим накопленные данные
    }
    catch (const std::exception& e)
    {}

	request.printRequest();
	//Response response(Response::FILE, 0, "", "", "/var/www/example");
	std::cerr << request.hostName << std::endl;
	ServerConfig& config = con.getConfig(request.hostName);
	Response response = Response::handleRequest(config, request);
	//response.print();

	//TODO if (connection == close)
	//	close(fd);

	const std::string http_response = response.toHttpResponse();
	// Отправка ответа клиенту
	send(con.poll.fd, http_response.c_str(), http_response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
	
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
