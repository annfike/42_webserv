#include "Server.hpp"

static SocketManager* sm = NULL;

int exit_requested = 0;

void handleSignal(int)
{
	//if (sm)
	//	sm->closeSockets();
	exit_requested = 1;
}

Server::Server(const std::string &config)
{
	parseConfig(config);
	sm = &socketManager;
	signal(SIGINT, handleSignal);
	signal(SIGQUIT, handleSignal);
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
		servers[i].locations[errLoc.path] = errLoc;
	}

	for (size_t i = 0; i < servers.size(); ++i)
	{
		std::cout << "-----------------Server " << i << ":----------------------------" << std::endl;
		servers[i].print();
	}
}

void Server::execRead(Connection& con)
{
	if (!socketManager.getConnection(con.poll.fd))
		return;

	std::cerr << "\n----------------- READING FROM FD=" << con.poll.fd << " -----------------" <<std::endl;
	
	// Чтение HTTP-запроса от клиента
	std::vector<char> accumulatedData;
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read;

	while ((bytes_read = recv(con.poll.fd, buffer, sizeof(buffer) - 1, 0)) > 0)
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

	std::cerr << "\n---> Request received: \n";
    for (size_t i = 0; i < std::min(accumulatedData.size(), (size_t)500); i++) {
	    std::cout << accumulatedData[i];
    }
	std::cerr << "\n---> End of Request\n\n";

	// Request & Response
	HttpRequestParser request;
	try
    {
        request.parse(accumulatedData);  // Парсим накопленные данные
    }
    catch (const std::exception& e) { return; }

	con.keepAlive = request.keepAlive;

	request.printRequest();

	std::cerr << request.hostName << std::endl;
	const ServerConfig& config = con.getConfig(request.hostName);
	Response response = Response::handleRequest(config, request);

	const std::string http_response = response.toHttpResponse(con.keepAlive, request.getMethod() == "HEAD");
	con.responseHeader = http_response;
	con.transferred = 0;
	con.poll.events = POLLOUT;
	//con.poll.events = POLLIN | POLLOUT;

	// Отправка ответа клиенту
	//send(con.poll.fd, http_response.c_str(), http_response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);

	//dodelat
	//con.keepAlive = false; // vse zapisali	
	//if (!con.keepAlive)
	//	socketManager.closeConnection(con);
}

void Server::execWrite(Connection& con)
{
	if (!socketManager.getConnection(con.poll.fd))
		return;

	std::size_t toSend = std::min(con.responseHeader.size() - con.transferred, BUFFER_SIZE);
	if (toSend != 0) 
	{
		send(con.poll.fd, con.responseHeader.c_str() + con.transferred, toSend, MSG_DONTWAIT | MSG_NOSIGNAL);
		con.transferred += toSend;
		//con.headerSent = true;
	}
	else
	{
		con.responseHeader.clear();
		con.transferred = 0;
		con.poll.events = POLLIN;
		if (!con.keepAlive)
			socketManager.closeConnection(con);
	}
	/*if (!con.file)
		return;

	char buf[BUFFER_SIZE];
	if (con.file->read(buf, sizeof(buf)) || con.file->gcount() > 0) {
		send(con.poll.fd, buf, con.file->gcount(), MSG_DONTWAIT | MSG_NOSIGNAL);
	}
	else {
		con.file->close();
		socketManager.closeConnection(con);
	}*/
}

void Server::print(std::vector<Connection*>& cons)
{
	std::vector<int> act;
	for (size_t i = 0; i < cons.size(); i++)
	{
		if (cons[i]->transferred == 0)
			act.push_back(i);
	}

	if (act.empty())
		return;

	socketManager.print();
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

	while (!exit_requested)
	{
		// Ожидаем активности на одном из сокетов
		std::vector<Connection*> cons = socketManager.getActiveConnections();
		if (cons.empty())
			continue;

		print(cons);
		for (size_t i = 0; i < cons.size(); i++)
		{
			if (cons[i]->isSocket)
			{
				socketManager.acceptConnection(*cons[i]);
				continue;
			}

			if (cons[i]->poll.revents & POLLIN)
				execRead(*cons[i]);
			if (cons[i]->poll.revents & POLLOUT)
				execWrite(*cons[i]);
		}
	}
}
