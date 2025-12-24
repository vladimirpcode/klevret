#pragma once

#include <vector>
#include <string>
#include "tcp_packet.hpp"


namespace common{


class TcpConnector{
public:
    TcpConnector(const std::string& server_ip, uint16_t port);
    void send_message(const tcp_packet message);
    ~TcpConnector();
private:
    int _socket;
    uint16_t _port;
};


}
