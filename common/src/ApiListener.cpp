#include "ApiListener.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "Defer.hpp"

#include <iostream>

namespace common{


ApiListener::ApiListener(uint16_t tcp_port)
    : _stop_flag(false),
    _tcp_port(tcp_port),
    _thread(new std::thread(_thread_func, this)),
    _client_socket(-1),
    _server_socket(-1)
{
    _server_socket = _create_socket_and_listen("127.0.0.1", _tcp_port, 1);
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    _client_socket = accept(_server_socket, (sockaddr*)&client_addr, &client_len);
    if (_client_socket == -1) {
        throw std::runtime_error("error client accepting");
    }

}


ApiListener::~ApiListener(){
    _stop_flag = true;
    _thread->join();
    close(_client_socket);
    close(_server_socket);
}

bool ApiListener::is_there_messages(){
    std::lock_guard<std::mutex> lg(_queue_mutex);
    return !_is_empty_no_mutex();
}

ApiMessage ApiListener::get_next_message(){
    std::lock_guard<std::mutex> lg(_queue_mutex);
    if (_is_empty_no_mutex()){
        throw std::runtime_error("Очередь пуста");
    }
    ApiMessage api_message = _queue.front();
    _queue.pop();
    return api_message;
}

void ApiListener::send_reply(ApiMessage& api_message){
    std::lock_guard<std::mutex> lg(_send_mutex);
    tcp_packet packet = api_message.serialize();
    std::cout << "отправляю ответ... \n";
    std::string s(packet.begin(), packet.end());
    std::cout << s << "\n";
    int sent_bytes = send(_client_socket, reinterpret_cast<const void *>(packet.data()), packet.size(), 0);
    if (sent_bytes <= -1){
        throw std::runtime_error("error sending reply");
    }
}

int ApiListener::_create_socket_and_listen(const std::string& ip, uint16_t port, int max_connections){
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        throw std::runtime_error("не могу создать сокет");
    }
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(server_socket);
        throw std::runtime_error("не удалось установить опцию REUSEADDR для сокета");
    }
    sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family =  AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) == -1){
        throw std::runtime_error("не удалось преобразовать IP адрес для sockaddr_in");
    }
    sa.sin_port = htons(port);

    if (bind(server_socket, (sockaddr*)&sa, sizeof sa) == -1){
        close(server_socket);
        throw std::runtime_error("не удалось забиндить сокет");
    }
    if (listen(server_socket, max_connections) == -1){
        close(server_socket);
        throw std::runtime_error("не удалось поставить сокет на прослушку");
    }
    return server_socket;
}

void ApiListener::_thread_func(ApiListener *listener){
    while (!listener->_stop_flag){
        const int BUFFER_SIZE = 1024*10;
        uint8_t buffer[BUFFER_SIZE];
        int bytes_read = recv(listener->_client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read == -1){
            continue;
        }
        if (bytes_read > 0) {
            tcp_packet packet(std::begin(buffer), std::begin(buffer) + bytes_read);
            ApiMessage api_message(packet);
            listener->_queue_mutex.lock();
            listener->_queue.push(api_message);
            listener->_queue_mutex.unlock();
        }
    }
}


bool ApiListener::_is_empty_no_mutex(){
    return _queue.empty();
}


}
