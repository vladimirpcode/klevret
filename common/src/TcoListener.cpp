#include "TcpListener.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

TcpListener::TcpListener(const std::string& ip, int port, int max_connections)
    : _port(port), _stop_flag(false),
    _max_connections(max_connections),
    _ip(ip)
{
    start();
}

TcpListener::~TcpListener(){
    stop();
}

tcp_packet TcpListener::get_next_packet(){
    std::lock_guard<std::mutex> guard(_queue_mutex);
    if (_is_empty_no_mutex()){
        throw std::runtime_error("Очередь пуста");
    }
    tcp_packet packet = _queue.front();
    _queue.pop();
    return packet;
}

bool TcpListener::is_empty(){
    std::lock_guard<std::mutex> guard(_queue_mutex);
    return _is_empty_no_mutex();
}

bool TcpListener::_is_empty_no_mutex(){
    return _queue.size() == 0;
}

void TcpListener::stop(){
    _stop_flag = true;
    _thread->join();
    delete _thread;
}


void TcpListener::start(){
    _stop_flag = false;
    _thread = new std::thread(_thread_func, this);
}


int TcpListener::_create_socket_and_listen(const std::string& ip, int port, int max_connections){
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

void TcpListener::_thread_func(TcpListener *listener){
    int server_socket = _create_socket_and_listen(listener->_ip, listener->_port, listener->_max_connections);
    std::vector<std::thread> client_handler_threads;
    while (!listener->_stop_flag){
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            continue;
        }
        client_handler_threads.push_back(std::thread(_client_handler_thread, listener, client_socket, client_addr));
    }
    for (auto& client_handler_thread : client_handler_threads){
        client_handler_thread.join();
    }
    close(server_socket);
}


void TcpListener::_client_handler_thread(TcpListener *listener, int client_socket, sockaddr_in client_addr){
    const int BUFFER_SIZE = 1024*10;
        uint8_t buffer[BUFFER_SIZE];
        while (!listener->_stop_flag){
            int bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0) {
                tcp_packet packet(std::begin(buffer), std::begin(buffer) + bytes_read);
                listener->_queue_mutex.lock();
                listener->_queue.push(packet);
                listener->_queue_mutex.unlock();
            }
        }
}
