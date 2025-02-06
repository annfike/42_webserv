#include "SocketManager.hpp"

SocketManager::SocketManager()
{
}

SocketManager::~SocketManager()
{
	closeSockets();
}

struct pollfd getPollFd(int fd)
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN ;//| POLLOUT;
	return pfd;
}

void SocketManager::bindSocket(int port)
{
	std::vector<int>::iterator it = std::find(ports.begin(), ports.end(), port);
	if (it != ports.end())
		return;

	std::cerr << "/"<<port << "/\n";

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
		throw std::runtime_error("Ошибка при создании сокета");

	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		//Ошибка сокета;
		std::cerr << std::strerror(errno) << std::endl;
		close(socket_fd);
		return;
	}
	
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	std::cerr << '|' << socket_fd << '-' << port << '|';
	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(socket_fd);
		std::cerr << std::strerror(errno) << std::endl;
		return;
		throw std::runtime_error("Ошибка при привязке сокета к адресу");
	}

	if (listen(socket_fd, 32) < 0)
	{
		close(socket_fd);
		throw std::runtime_error("Ошибка при переводе сокета в режим прослушивания");
	}
	//fcntl(socket_fd, F_SETFL, O_NONBLOCK);

	sockets.push_back(getPollFd(socket_fd));
	ports.push_back(port);
}

bool SocketManager::getActive(std::vector<struct pollfd>& fds)
{
	if (fds.empty())
		return false;

	std::cerr << "poll\n";
	int activity = poll(fds.data(), fds.size(), -1);
	if (activity <= 0)
		return false;
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

	std::cout << "Новое соединение принято (" << client_fd << ")" << std::endl;
	return getPollFd(client_fd);
}

void SocketManager::closeFd(int fd)
{
	close(fd);  // Закрытие соединения с клиентом
	//FD_CLR(fd, &master_set);  // Удаляем клиентский сокет из master_set
	//std::remove(reads.begin(), reads.end(), fd);
}

bool SocketManager::isSocket(int fd)
{
	for (size_t i = 0; i < sockets.size(); i++)
	{
		if (sockets[i].fd == fd)
			return true;
	}
	return false;
}

void SocketManager::closeSockets()
{
	for (size_t i = 0; i < sockets.size(); i++)
	{
		close(sockets[i].fd);
	}
	sockets.clear();
}