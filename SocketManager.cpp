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
	pfd.events = POLLIN ;//| POLLOUT;
	return pfd;
}

void SocketManager::bindSocket(ServerConfig config)
{
	std::string ip = config.listen_IP;
	int port = std::atoi(config.listen.c_str());
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i].ip == ip && connections[i].port == port)
			return;
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
	con.config = config;
	connections.push_back(con);
}

bool SocketManager::getActive(std::vector<struct pollfd>& fds)
{
	if (fds.empty())
		return false;

	std::cerr << "poll\n";
	int activity = poll(fds.data(), fds.size(), -1);
	if (activity <= 0)
	{
		std::cerr << activity << " poll fail\n";
		return false;
	}
	std::cerr << activity << "poll true\n";
	return true;

	/*
	fdread = master_set;
	fdwrite = master_set;

	int activity = select(max_fd + 1, &fdread, &fdwrite, NULL, 0);
	if (activity == 0)
		return false;
	if (activity == -1)
	{
		std::cerr << std::strerror(errno) << std::endl;
		std::cerr << "Ошибка при вызове select!" << std::endl;
		return false;
	}
	return true;
	*/
}

struct pollfd SocketManager::acceptConnection(struct pollfd socket)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// Принятие нового соединения
	int client_fd = accept(socket.fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0)
	{
		std::cerr << "Ошибка при принятии соединения!" << std::endl;
		return getPollFd(-1);
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	//fcntl(client_fd, F_SETFD, FD_CLOEXEC);

	std::cout << std::endl;
	std::cout << "New client connection on socket " << socket.fd << " (fd=" << client_fd << ")" << std::endl;
	std::cout << std::endl;
	
	return getPollFd(client_fd);
}

bool SocketManager::isSocket(int fd)
{
	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i].poll.fd == fd)
			return true;
	}
	return false;
}

void SocketManager::closeSockets()
{
	for (size_t i = 0; i < connections.size(); i++)
	{
		close(connections[i].poll.fd);
	}
	connections.clear();
}