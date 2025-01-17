#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    // Создание сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Ошибка при создании сокета!" << std::endl;
        return 1;
    }

    // Настройка адреса и порта для сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Принимаем запросы с любых IP-адресов
    server_addr.sin_port = htons(8080); // Порт сервера

    // Привязка сокета к адресу и порту
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Ошибка при привязке сокета!" << std::endl;
        return 1;
    }

    // Прослушивание порта
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Ошибка при прослушивании порта!" << std::endl;
        return 1;
    }

    std::cout << "Сервер запущен на порту 8080..." << std::endl;

    // Серверный цикл
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Принятие соединения от клиента
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Ошибка при принятии соединения!" << std::endl;
            continue;
        }

        // Чтение HTTP-запроса от клиента
        char buffer[1024];
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            std::cerr << "Ошибка при чтении данных!" << std::endl;
            close(client_fd);
            continue;
        }
        buffer[bytes_read] = '\0'; // Завершаем строку
        std::cout << "Получен запрос: \n" << buffer << std::endl;

        // Формирование HTTP-ответа
        const char* http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>Hello, World!</h1></body></html>";

        // Отправка ответа клиенту
        write(client_fd, http_response, strlen(http_response));

        // Закрытие соединения с клиентом
        close(client_fd);
    }

    // Закрытие сокета сервера (эта строка не будет достигнута, так как сервер работает в бесконечном цикле)
    close(server_fd);
    return 0;
}

/*
Шаги для компиляции и запуска:
Сохраните код в файл. Назовите его, например, http_server.cpp.
Компиляция:
Откройте терминал.
Перейдите в директорию, где сохранён файл http_server.cpp.
Скомпилируйте код с помощью компилятора g++:
bash
Copy code
g++ -o http_server http_server.cpp
Эта команда создаст исполняемый файл с именем http_server.
Запуск:
Для запуска сервера выполните команду:
bash
Copy code
./http_server
После этого сервер будет слушать на порту 8080.
Проверка работы сервера:
Откройте браузер или используйте команду curl, чтобы проверить работу сервера.
Введите в браузере URL:
arduino
Copy code
http://localhost:8080
Или используйте команду curl:

bash
Copy code
curl http://localhost:8080
Сервер должен ответить HTML-страницей:

css
Copy code
<html><body><h1>Hello, World!</h1></body></html>
Примечания:
Сервер будет работать в бесконечном цикле, принимая соединения на порту 8080 и отвечая на каждый запрос.
В этом примере сервер обрабатывает только базовые HTTP-запросы (метод GET), не поддерживает никаких дополнительных функций, таких как обработка разных URL или многозадачность.
Если вы хотите изменить порт, просто измените строку server_addr.sin_port = htons(8080); на любой другой номер порта.
Если вы используете Windows, код потребуется немного адаптировать, так как Windows использует свой API для работы с сокетами (Winsock).

*/