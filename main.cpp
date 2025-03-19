#include "Server.hpp"

int main(int argc, char **argv) 
{
	std::string config = "simple.conf";
	if (argc == 2)
		config = argv[1];
	signal(SIGPIPE, SIG_IGN);
	Server server(config);
    server.loop();
}

/*std::string method = "DELETE"; // для теста
    // Обработка метода DELETE
    if (type == FILE && code == 200 &&  method == "DELETE")

    {
        if (std::remove(filePath.c_str()) == 0) { // Удаление файла
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/plain\r\n";
            response << "Connection: close\r\n\r\n";
            response << "File deleted successfully.";
        } else {
            response << "HTTP/1.1 500 Internal Server Error\r\n";
            response << "Content-Type: text/plain\r\n";
            response << "Connection: close\r\n\r\n";
            response << "Failed to delete file.";
        }

        std::string responseStr = response.str();
        char* httpResponse = new char[responseStr.size() + 1];
        std::strcpy(httpResponse, responseStr.c_str());
        return httpResponse;
    }
    проверка curl -X DELETE http://localhost:8080/file.txt
    */