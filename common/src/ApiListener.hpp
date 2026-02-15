#pragma once

#include <cstdint>
#include "ApiMessage.hpp"
#include <queue>
#include <thread>
#include <memory>
#include <atomic>

namespace common{


class ApiListener{
public:
    ApiListener(uint16_t tcp_port);
    ~ApiListener();
    bool is_there_messages();
    ApiMessage get_next_message();
    void send_reply(ApiMessage& api_message);
private:
    std::unique_ptr<std::thread> _thread;
    std::atomic<bool> _stop_flag;
    std::queue<ApiMessage> _queue;
    std::mutex _queue_mutex;
    std::mutex _send_mutex;
    int _client_socket;
    int _server_socket;
    uint16_t _tcp_port;
    static int _create_socket_and_listen(const std::string& ip, uint16_t port, int max_connections);
    static void _thread_func(ApiListener *listener);
    bool _is_empty_no_mutex();
};


}
