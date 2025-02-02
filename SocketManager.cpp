#include "SocketManager.hpp"

SocketManager::SocketManager()
{
	max_fd = 0;
	FD_ZERO(&master_set);  // Обнуляем master_set	
}

SocketManager::~SocketManager()
{
	for (size_t i = 0; i < sockets.size(); i++)
	{
		close(sockets[i]);
	}	
}

void SocketManager::bindSocket(int port)
{
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		throw std::runtime_error("Ошибка при создании сокета");
	}

	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	std::cerr << '|' << socket_fd << '-' << port << '|';
	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		throw std::runtime_error("Ошибка при привязке сокета к адресу");
	}

	if (listen(socket_fd, 10) == -1)
	{
		throw std::runtime_error("Ошибка при переводе сокета в режим прослушивания");
	}

	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		//Ошибка сокета;
		std::cerr << std::strerror(errno) << std::endl; 
		return;
	}

	FD_SET(socket_fd, &master_set);
	sockets.push_back(socket_fd);
	if (max_fd < socket_fd)
		max_fd = socket_fd;
}

bool SocketManager::isActive(fd_set& fdread, fd_set& fdwrite)
{
	fdread = master_set;
	fdwrite = master_set;
	int activity = select(max_fd + 1, &fdread, &fdwrite, NULL, NULL);
	if (activity == -1)
	{
		std::cerr << std::strerror(errno) << std::endl;
		std::cerr << "Ошибка при вызове select!" << std::endl;
		return false;
	}
	return true;
}

bool SocketManager::acceptConnection(int socket)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	// Принятие нового соединения
	int client_fd = accept(socket, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) 
	{
		std::cerr << "Ошибка при принятии соединения!" << std::endl;
		return false;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	//fcntl(client_fd, F_SETFD, FD_CLOEXEC);

	// Добавляем клиентский сокет в master_set
	FD_SET(client_fd, &master_set);
	if (max_fd < client_fd) 
		max_fd = client_fd;  // Обновляем максимальный дескриптор

	std::cout << "Новое соединение принято" << std::endl;

	return false;
}

int SocketManager::getMax_fd()
{
	return max_fd;
}

void SocketManager::close(int fd)
{
	close(fd);  // Закрытие соединения с клиентом
	FD_CLR(fd, &master_set);  // Удаляем клиентский сокет из master_set
}

bool SocketManager::isSocket(int fd)
{
	for (size_t i = 0; i < sockets.size(); i++)
	{
		if (sockets[i] == fd)
			return true;
	}
	return false;
}