#include "SocketManager.hpp"
#include <fstream>

SocketManager::SocketManager()
{
}

SocketManager::SocketManager(const SocketManager &s)
{
	(*this) = s;
}

SocketManager::~SocketManager()
{
	closeSockets();
}

SocketManager &SocketManager::operator=(const SocketManager &s)
{
	this->connections = s.connections;
	return *this;
}

struct pollfd getPollFd(int fd)
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN | POLLOUT;
	return pfd;
}

void SocketManager::bindSocket(ServerConfig config)
{
	std::string ip = config.listen_IP;
	int port = std::atoi(config.listen.c_str());
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i].ip == ip && connections[i].port == port)
		{
			connections[i].configs.push_back(config);
			return;
		}
	}

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
		throw std::runtime_error("Ошибка при создании сокета");

	//allow reuse closed socket
	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		//Ошибка сокета;
		std::cerr << std::strerror(errno) << std::endl;
		close(socket_fd);
		return;
	}

	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(ip.c_str(), NULL, &hints, &res) != 0)
	{
		std::cerr << "Invalid IP address: " << ip << std::endl;
		close(socket_fd);
		return;
	}

	struct sockaddr_in server_addr;
	std::memcpy(&server_addr, res->ai_addr, sizeof(struct sockaddr_in));
	server_addr.sin_port = htons(port);
	freeaddrinfo(res);

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		//Ошибка при привязке сокета к адресу
		close(socket_fd);
		std::cerr << std::strerror(errno) << std::endl;
		return;
	}

	if (listen(socket_fd, 32) < 0)
	{
		//Ошибка при переводе сокета в режим прослушивания
		close(socket_fd);
		std::cerr << std::strerror(errno) << std::endl;
		return;
	}
	//fcntl(socket_fd, F_SETFL, O_NONBLOCK);

	std::cout << "	" << ip << ":" << port << " (" << socket_fd << ")" << std::endl;

	Connection con;
	con.ip = ip;
	con.port = port;
	con.poll = getPollFd(socket_fd);
	con.isSocket = true;
	con.configs.push_back(config);
	connections.push_back(con);
}

std::vector<Connection> SocketManager::getActiveConnections()
{
	std::vector<struct pollfd> fds;
	for (size_t i = 0; i < connections.size(); i++)
	{
		fds.push_back(connections[i].poll);
	}

	if (fds.empty())
		return std::vector<Connection>();

	int activity = poll(fds.data(), fds.size(), -1);
	if (activity <= 0)
	{
		std::cerr << std::strerror(errno) << std::endl;
		return std::vector<Connection>();
	}

	std::vector<Connection> cons;
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (!fds[i].revents)
			continue;
		
		Connection* con = getConnection(fds[i].fd);
		con->poll.revents = fds[i].revents;
		cons.push_back(*con);
	}

	return cons;
}

void SocketManager::acceptConnection(Connection socket)
{
	if (!socket.isSocket)
	{
		std::cerr << "Connection not found - " << socket.poll.fd << std::endl;
		return;
	}

	// Принятие нового соединения
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(socket.poll.fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0)
	{
		std::cerr << std::strerror(errno) << std::endl;
		std::cerr << "Ошибка при принятии соединения!" << std::endl;
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	//fcntl(client_fd, F_SETFD, FD_CLOEXEC);

	std::cout << std::endl;
	std::cout << "New client connection on socket " << socket.poll.fd << " (fd=" << client_fd << ")" << std::endl;
	std::cout << std::endl;

	Connection con;
	con.isSocket = false;
	con.ip = socket.ip;
	con.port = socket.port;
	con.configs = socket.configs;
	con.poll = getPollFd(client_fd);
	connections.push_back(con);
}

void SocketManager::closeSockets()
{
	for (size_t i = 0; i < connections.size(); i++)
	{
		close(connections[i].poll.fd);
	}
	connections.clear();
}

Connection* SocketManager::getConnection(int fd)
{
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i].poll.fd == fd)
			return &connections[i];
	}
	return NULL;
}

void SocketManager::closeConnection(Connection con)
{
	for (size_t j = 0; j < connections.size(); j++) 
	{
		if (connections[j].poll.fd == con.poll.fd) 
		{
			//shutdown(con.poll.fd, SHUT_WR);
			close(con.poll.fd);
			connections.erase(connections.begin() + j);
			
			std::cout << std::endl;
			std::cout << "Client disconnected" << " (fd=" << con.poll.fd << ")" << std::endl;
			std::cout << std::endl;

			break;
		}
	}
}