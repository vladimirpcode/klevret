#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <netinet/in.h>
#include "tcp_packet.hpp"



namespace common{



class TcpListener{
public:
    TcpListener(const std::string& ip, int port, int max_connections);
    ~TcpListener();
    tcp_packet get_next_packet();
    bool is_empty();
    void stop();
    void start();
private:
    const std::string _ip;
    const int _max_connections;
    const int _port;
    std::queue<tcp_packet> _queue;
    std::mutex _queue_mutex;
    std::thread *_thread;
    std::atomic<bool> _stop_flag;
    static int _create_socket_and_listen(const std::string& ip, int port, int max_connections);
    static void _thread_func(TcpListener *listener);
    static void _client_handler_thread(TcpListener *listener, int client_socket, sockaddr_in client_addr);
    bool _is_empty_no_mutex();
};



}
