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

void Server::execRead(int fd)
{
	Connection& con = *socketManager.getConnection(fd);
	if (con.closed)
		return;

	std::cerr << "\n----------------- READING FROM FD=" << con.poll.fd << " -----------------" <<std::endl;

	// Чтение HTTP-запроса от клиента
	char buffer[BUFFER_SIZE];
	// Добавляем в накопленные данные
	ssize_t bytes_read = recv(con.poll.fd, buffer, sizeof(buffer), 0);
	if (bytes_read <= 0)
	{
		// Если ошибка при чтении или клиент закрыл соединение
		if (bytes_read < 0)
			std::cerr << "Error reading request body: " << strerror(errno) << std::endl;
		con.closed = true;
		return;
	}

	con.requestBuffer.insert(con.requestBuffer.end(), buffer, buffer + bytes_read);
	std::string tmp(con.requestBuffer.begin(), con.requestBuffer.end());
	std::size_t rn = tmp.find("\r\n\r\n");
	if (rn == std::string::npos)
		return;

	HttpRequestParser request;
	try
    {
		if (request.headers.size() == 0)
		    request.parse(con.requestBuffer, rn);

		std::map<std::string, std::string>::const_iterator it = request.headers.find("Content-Length");
		std::map<std::string, std::string>::const_iterator chunked = request.headers.find("Transfer-Encoding");
		if (it != request.headers.end())
		{
			unsigned long contentLength = std::strtoul(it->second.c_str(), NULL, 10);
			if (contentLength != 0)
			{
				if (con.requestBuffer.size() < rn + 4 + contentLength)
					return;
				
				request.parseBody(con.requestBuffer, rn);
			}
		}
		else if (chunked != request.headers.end() && chunked->second == "chunked")
		{
			if (!request.parseChunkedBody(con.requestBuffer, rn))
				return;
		}
    }
    catch (const std::exception& e) {
		return;
	}

	/*std::cerr << "\n---> Request received: \n";
    for (std::size_t i = 0; i < std::min(con.requestBuffer.size(), (size_t)500); i++) {
	    std::cout << con.requestBuffer[i];
    }
	std::cerr << "\n---> End of Request\n\n";
	*/
	
	//request.printRequest();

	con.requestBuffer.clear();
	con.keepAlive = request.keepAlive;

	//std::cerr << request.hostName << std::endl;
	const ServerConfig& config = con.getConfig(request.hostName);
	Response response = Response::handleRequest(config, request);

	con.responseHeader = response.toHttpResponse(con.keepAlive, request.getMethod() == "HEAD");
	con.transferred = 0;
	con.poll.events = POLLOUT;
	con.poll.revents = 0;
}

void Server::execWrite(int fd)
{
	Connection& con = *socketManager.getConnection(fd);
	if (con.closed)
		return;
	std::size_t toSend = std::min(con.responseHeader.size() - con.transferred, BUFFER_SIZE);
	if (toSend != 0) 
	{
		ssize_t bytes_send = send(con.poll.fd, con.responseHeader.c_str() + con.transferred, toSend, MSG_DONTWAIT | MSG_NOSIGNAL);
		if (bytes_send <= 0)
		{
			std::cerr << "Error sending response body: " << strerror(errno) << std::endl;
			con.closed = true;
			return;
		}
		
		con.transferred += toSend;
	}
	else
	{
		con.responseHeader.clear();
		con.transferred = 0;
		con.poll.events = POLLIN;
		con.poll.revents = 0;
		if (!con.keepAlive)
			con.closed = true;
	}
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
				execRead(cons[i]->poll.fd);
			if (cons[i]->poll.revents & POLLOUT)
				execWrite(cons[i]->poll.fd);
		}
	}
}
