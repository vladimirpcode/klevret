#include "api_server.hpp"


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <mutex>
#include <queue>


class TcpServer{
public:
    TcpServer(){
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket == -1){
            throw std::runtime_error("не могу создать сокет");
        }
        int opt = 1;
        if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            close(_socket);
            throw std::runtime_error("не удалось установить опцию REUSEADDR для сокета");
        }
        sockaddr_in sa;
        memset(&sa, 0, sizeof sa);
        sa.sin_family =  AF_INET;
        if (inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr) == -1){
            throw std::runtime_error("не удалось преобразовать IP адрес для sockaddr_in");
        }
        sa.sin_port = htons(_port);

        if (bind(_socket, (sockaddr*)&sa, sizeof sa) == -1){
            close(_socket);
            throw std::runtime_error("не удалось забиндить сокет");
        }
        if (listen(_socket, 5) == -1){
            close(_socket);
            throw std::runtime_error("не удалось поставить сокет на прослушку");
        }
        _thread = new std::thread(acceptor, _socket);

    }
    ~TcpServer(){
        close(_socket);
        delete _thread;
    }
    void join_threads(){
        _thread->join();
    }
    void stop(){
        // ToDo
    }
private:
    std::thread *_thread;
    int _socket;
    const int _port = 40237;
    std::queue<std::string> _messages;
    static void acceptor(int server_socket){
        std::cout << "начал прием TCP соединений" << "\n";
        std::vector<std::thread> client_handler_threads;
        while(true){
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
            if (client_socket == -1) {
                std::cout << "не удалось принять подключение\n";
                continue;
            }
            client_handler_threads.push_back(std::thread(client_handler_thread, client_socket));
        }
    }
    static void client_handler_thread(int client_socket){
        std::cout << "запущен обработчик клиента\n";
        const int BUFFER_SIZE = 1024*10;
        char buffer[BUFFER_SIZE];
        while (true){
            int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "Received: \n" << buffer << "\n\n";

                // Отправляем ответ
                //const char* response = "Message received";
                //write(client_socket, response, strlen(response));
            }
        }
    }
};


ApiServer *ApiServer::_api_server;
std::mutex ApiServer::_mutex;

ApiServer& ApiServer::instance(){
    if (!_api_server){
        _mutex.lock();
        if (!_api_server){
            _api_server = new ApiServer();
        }
    }
    return *_api_server;
}


void ApiServer::start(){
    TcpServer tcp_server;
    tcp_server.join_threads();
}


// ToDo заменить на общий TcpListener. Кажется, что я думал о возможности подключать несколько клиеннтов к серверу
// таких как CLI и фронтенд, но думаю, что нужно положить всю ответственность на Core
// и чтобы DHCP сервер держал одно подключение. Либо написать один на все компоненты ApiServer
// чтобы DHCP мог управляться вне Core
// Вопрос: может ли сервер управляться в обход Core или нет
// Вопрос: решена ли проблема отправки с конкретного интерфейса?
// Вопрос: решена ли проблема отправки на конкретный MAC адррес вместо широковещательной IP рассылки, что
//         собственно требуется стандартом DHCP

// Todo: перевести проект на английский язык
